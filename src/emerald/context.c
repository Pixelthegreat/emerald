/*
 * Copyright 2025, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <emerald/core.h>
#include <emerald/memory.h>
#include <emerald/utf8.h>
#include <emerald/wchar.h>
#include <emerald/hash.h>
#include <emerald/path.h>
#include <emerald/string.h>
#include <emerald/map.h>
#include <emerald/list.h>
#include <emerald/none.h>
#include <emerald/function.h>
#include <emerald/class.h>
#include <emerald/context.h>

/* visitors */
static em_value_t (*visitors[EM_NODE_TYPE_COUNT])(em_context_t *, em_node_t *) = {
	[EM_NODE_TYPE_BLOCK] = em_context_visit_block,
	[EM_NODE_TYPE_INT] = em_context_visit_int,
	[EM_NODE_TYPE_FLOAT] = em_context_visit_float,
	[EM_NODE_TYPE_STRING] = em_context_visit_string,
	[EM_NODE_TYPE_IDENTIFIER] = em_context_visit_identifier,
	[EM_NODE_TYPE_LIST] = em_context_visit_list,
	[EM_NODE_TYPE_MAP] = em_context_visit_map,
	[EM_NODE_TYPE_UNARY_OPERATION] = em_context_visit_unary_operation,
	[EM_NODE_TYPE_BINARY_OPERATION] = em_context_visit_binary_operation,
	[EM_NODE_TYPE_ACCESS] = em_context_visit_access,
	[EM_NODE_TYPE_CALL] = em_context_visit_call,
	[EM_NODE_TYPE_CONTINUE] = em_context_visit_continue,
	[EM_NODE_TYPE_BREAK] = em_context_visit_break,
	[EM_NODE_TYPE_RETURN] = em_context_visit_return,
	[EM_NODE_TYPE_INCLUDE] = em_context_visit_include,
	[EM_NODE_TYPE_LET] = em_context_visit_let,
	[EM_NODE_TYPE_IF] = em_context_visit_if,
	[EM_NODE_TYPE_FOR] = em_context_visit_for,
	[EM_NODE_TYPE_FOREACH] = em_context_visit_foreach,
	[EM_NODE_TYPE_WHILE] = em_context_visit_while,
	[EM_NODE_TYPE_FUNC] = em_context_visit_func,
	[EM_NODE_TYPE_CLASS] = em_context_visit_class,
	[EM_NODE_TYPE_PUTS] = em_context_visit_puts,
};

/* NOTE: Always leave pathbuf1 for reuse, even if used previously */
#define PATHBUFSZ 4096
static char pathbuf1[PATHBUFSZ];
static char pathbuf2[PATHBUFSZ];

/* create context */
EM_API em_context_t *em_context_new(const char **argv) {

	em_context_t *context = em_malloc(sizeof(em_context_t));
	if (!context) return NULL;

	if (em_context_init(context, argv) != EM_RESULT_SUCCESS) {

		em_context_free(context);
		return NULL;
	}

	return context;
}

/* initialize context */
EM_API em_result_t em_context_init(em_context_t *context, const char **argv) {

	if (!context) return EM_RESULT_FAILURE;
	else if (context->init) {

		em_log_fatal("Context already initialized");
		return EM_RESULT_FAILURE;
	}

	context->argv = argv;
	context->lexer = EM_LEXER_INIT;
	context->parser = EM_PARSER_INIT;

	if (em_lexer_init(&context->lexer) != EM_RESULT_SUCCESS)
		return EM_RESULT_FAILURE;
	if (em_parser_init(&context->parser) != EM_RESULT_SUCCESS)
		return EM_RESULT_FAILURE;

	/* set up initial directory stack */
	context->ndirstack = 2;
	context->dirstack[0] = ".";
	context->dirstack[1] = EM_STDLIB_DIR;
	context->nscopestack = 1;
	context->scopestack[0] = em_map_new();
	em_value_incref(context->scopestack[0]);
	context->rec_first = NULL;
	context->rec_last = NULL;
	context->pass = EM_VALUE_FAIL;

	context->init = EM_TRUE;
	return EM_RESULT_SUCCESS;
}

/* run code */
static void remove_text(const char *text) {

	em_node_t *node = EM_NODE(em_reflist_node.normal.first);
	while (node) {

		if (node->pos.text == text) node->pos.text = NULL;
		node = EM_NODE(EM_REFOBJ(node)->next);
	}

	em_token_t *token = EM_TOKEN(em_reflist_token.normal.first);
	while (token) {

		if (token->pos.text == text) token->pos.text = NULL;
		token = EM_TOKEN(EM_REFOBJ(token)->next);
	}
}

EM_API em_value_t em_context_run_text(em_context_t *context, const char *path, const char *text, em_ssize_t len) {

	if (!context || !context->init) return EM_VALUE_FAIL;

	em_lexer_reset(&context->lexer, path, text, len);
	if (em_lexer_make_tokens(&context->lexer) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	em_parser_reset(&context->parser, context->lexer.first);
	if (em_parser_parse(&context->parser) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	em_node_t *node = context->parser.node;
	EM_NODE_INCREF(node);

	em_value_t result = em_context_visit(context, node);

	EM_NODE_DECREF(node);

	remove_text(text);
	return result;
}

/* push directory to stack */
EM_API const char *em_context_pushdir(em_context_t *context, const char *path) {

	if (!context || !context->init) return NULL;

	/* no space */
	if (context->ndirstack >= EM_CONTEXT_MAX_DIRS) {

		em_log_fatal("Reached directory stack limit");
		return NULL;
	}

	context->dirstack[context->ndirstack++] = path;
	return path;
}

/* resolve file path */
EM_API const char *em_context_resolve(em_context_t *context, const char *path) {

	if (!context || !context->init) return NULL;

	/* check directories */
	for (size_t i = 0; i < context->ndirstack; i++) {

		if (em_path_join(pathbuf1, PATHBUFSZ, 2, context->dirstack[i], path) != EM_RESULT_SUCCESS)
			return NULL;
		
		if (em_path_exists(pathbuf1)) {

			if (em_path_fix(pathbuf2, PATHBUFSZ, pathbuf1) != EM_RESULT_SUCCESS)
				return NULL;
			return pathbuf2;
		}
	}
	return NULL;
}

/* pop directory from stack */
EM_API const char *em_context_popdir(em_context_t *context) {

	if (!context || !context->init) return NULL;

	/* already at bottom */
	if (!context->ndirstack) {

		em_log_fatal("Reached bottom of directory stack");
		return NULL;
	}

	return context->dirstack[--context->ndirstack];
}

/* push scope to stack */
EM_API em_result_t em_context_push_scope(em_context_t *context) {

	if (!context || !context->init) return EM_RESULT_FAILURE;

	/* no space */
	if (context->nscopestack >= EM_CONTEXT_MAX_SCOPE) {

		em_log_fatal("Reached scope stack limit");
		return EM_RESULT_FAILURE;
	}

	em_value_t map = em_map_new();
	em_value_incref(map);

	context->scopestack[context->nscopestack++] = map;
	return EM_RESULT_SUCCESS;
}

/* pop scope from stack */
EM_API void em_context_pop_scope(em_context_t *context) {

	if (!context || !context->init) return;

	/* already at bottom */
	if (!context->nscopestack) {

		em_log_warning("Reached bottom of scope stack");
		return;
	}

	em_value_t map = context->scopestack[--context->nscopestack];
	em_value_decref(map);
}

/* set value in current scope */
EM_API void em_context_set_value(em_context_t *context, em_hash_t key, em_value_t value) {

	if (!context || !context->init || !context->nscopestack) return;

	em_value_t map = context->scopestack[context->nscopestack-1];
	em_map_set(map, key, value);
}

/* get value from current scope */
EM_API em_value_t em_context_get_value(em_context_t *context, em_hash_t key) {

	if (!context || !context->init || !context->nscopestack) return EM_VALUE_FAIL;
	for (size_t i = context->nscopestack; i > 0; i--) {

		em_value_t value = em_map_get(context->scopestack[i-1], key);
		if (EM_VALUE_OK(value)) return value;
	}
	return EM_VALUE_FAIL;
}

/* run code from file */
EM_API em_value_t em_context_run_file(em_context_t *context, em_pos_t *pos, const char *path) {

	if (!context || !context->init) return EM_VALUE_FAIL;

	/* get file path */
	const char *rpath = em_context_resolve(context, path);
	if (!rpath) {

		em_log_runtime_error(pos, "No such file or directory: '%s'", path);
		return EM_VALUE_FAIL;
	}

	/* check if file has already been run */
	em_recfile_t *recfile = context->rec_first;
	while (recfile) {
		if (!strcmp(recfile->rpath, rpath))
			return em_none;
		recfile = recfile->next;
	}

	/* add directory to directory stack */
	if (em_path_dirname(pathbuf1, PATHBUFSZ, rpath) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	char *buf = NULL;
	if (pathbuf1[0]) {

		size_t len = strlen(pathbuf1);
		buf = (char *)em_malloc(len+1);
		memcpy(buf, pathbuf1, len);
		buf[len] = 0;

		if (!em_context_pushdir(context, buf)) {

			em_free(buf);
			return EM_VALUE_FAIL;
		}
	}

	/* read file */
	FILE *fp = fopen(rpath, "rb");
	if (!fp) {

		em_log_runtime_error(pos, "%s: '%s'", strerror(errno), path);
		return EM_VALUE_FAIL;
	}

	fseek(fp, 0, SEEK_END);
	size_t len = (size_t)ftell(fp);
	fseek(fp, 0, SEEK_SET);

	char *fbuf = (char *)em_malloc(len+1);
	fread(fbuf, 1, len, fp);
	fbuf[len] = 0;
	fclose(fp);

	/* make file record */
	size_t reclen = strlen(rpath);
	recfile = em_malloc(sizeof(em_recfile_t)+reclen+1);
	memcpy(recfile->rpath, rpath, reclen);
	recfile->rpath[reclen] = 0;
	recfile->next = NULL;

	/* run code */
	em_value_t result = em_context_run_text(context, recfile->rpath, fbuf, (em_ssize_t)len);

	/* clean up */
	em_free(fbuf);
	if (buf) {

		(void)em_context_popdir(context);
		em_free(buf);
	}

	/* add file to run list */
	if (!context->rec_first) context->rec_first = recfile;
	if (context->rec_last) context->rec_last->next = recfile;
	context->rec_last = recfile;
	
	return result;
}

/* visit node */
EM_API em_value_t em_context_visit(em_context_t *context, em_node_t *node) {

	if (!visitors[node->type]) {

		em_log_runtime_error(&node->pos, "Unsupported node ('%s')", em_get_node_type_name(node->type));
		return EM_VALUE_FAIL;
	}
	return visitors[node->type](context, node);
}

/* visit block */
EM_API em_value_t em_context_visit_block(em_context_t *context, em_node_t *node) {

	em_node_t *cur = node->first;
	em_value_t result = em_none;
	while (cur) {

		em_value_delete(result);

		result = em_context_visit(context, cur);
		if (!EM_VALUE_OK(result)) return EM_VALUE_FAIL;

		cur = cur->next;
	}
	return result;
}

/* visit integer */
EM_API em_value_t em_context_visit_int(em_context_t *context, em_node_t *node) {

	em_token_t *token = em_node_get_token(node, 0);

	em_inttype_t value;
	sscanf(token->value, EM_INTTYPE_FORMAT, &value);

	return EM_VALUE_INT(value);
}

/* visit float */
EM_API em_value_t em_context_visit_float(em_context_t *context, em_node_t *node) {

	em_token_t *token = em_node_get_token(node, 0);

	em_floattype_t value;
	sscanf(token->value, EM_FLOATTYPE_FORMAT, &value);

	return EM_VALUE_FLOAT(value);
}

/* visit string */
EM_API em_value_t em_context_visit_string(em_context_t *context, em_node_t *node) {

	em_token_t *token = em_node_get_token(node, 0);
	return em_string_new_from_utf8(token->value, em_utf8_strlen(token->value));
}

/* visit identifier */
EM_API em_value_t em_context_visit_identifier(em_context_t *context, em_node_t *node) {

	em_token_t *token = em_node_get_token(node, 0);

	em_hash_t key = em_utf8_strhash(token->value);
	em_value_t value = em_context_get_value(context, key);

	if (!EM_VALUE_OK(value)) {

		em_log_runtime_error(&node->pos, "Variable '%s' not defined", token->value);
		return EM_VALUE_FAIL;
	}
	return value;
}

/* visit list */
EM_API em_value_t em_context_visit_list(em_context_t *context, em_node_t *node) {

	size_t nchildren = 0;

	em_node_t *cur = node->first;
	while (cur) {

		nchildren++;
		cur = cur->next;
	}
	cur = node->first;

	em_value_t list = em_list_new(nchildren);
	while (cur) {

		em_value_t value = em_context_visit(context, cur);
		if (!EM_VALUE_OK(value)) {

			em_value_delete(list);
			return EM_VALUE_FAIL;
		}
		em_list_append(list, value);
		cur = cur->next;
	}
	return list;
}

/* visit map */
EM_API em_value_t em_context_visit_map(em_context_t *context, em_node_t *node) {

	em_node_t *cur = node->first;
	em_value_t map = em_map_new();

	while (cur) {

		em_node_t *key_node = cur;
		em_node_t *value_node = key_node->next;

		em_value_t key = em_context_visit(context, key_node);
		if (!EM_VALUE_OK(key)) {

			em_value_delete(map);
			return EM_VALUE_FAIL;
		}
		em_value_t value = em_context_visit(context, value_node);
		if (!EM_VALUE_OK(value)) {

			em_value_delete(key);
			em_value_delete(map);
			return EM_VALUE_FAIL;
		}

		/* set item */
		em_hash_t hash = em_value_hash(key, &key_node->pos);
		em_map_set(map, hash, value);

		em_value_delete(key);
		cur = value_node->next;
	}
	return map;
}

/* visit unary operation */
EM_API em_value_t em_context_visit_unary_operation(em_context_t *context, em_node_t *node) {

	em_token_t *token = em_node_get_token(node, 0);
	em_node_t *right_node = node->first;

	em_value_t right = em_context_visit(context, right_node);
	if (!EM_VALUE_OK(right)) return EM_VALUE_FAIL;

	/* operation */
	em_value_t result;

	if (!strcmp(token->value, "+"))
		result = right;
	else if (!strcmp(token->value, "-"))
		result = em_value_multiply(right, EM_VALUE_INT(-1), &node->pos);
	else if (!strcmp(token->value, "not"))
		result = EM_VALUE_INT_INV(em_value_is_true(right, &node->pos));
	else {
		em_log_runtime_error(&node->pos, "Unsupported operation ('%s')", token->value);
		result = EM_VALUE_FAIL;
	}

	em_value_delete(right);
	return result;
}

/* visit binary operation */
EM_API em_value_t em_context_visit_binary_operation(em_context_t *context, em_node_t *node) {

	em_node_t *left_node = node->first;
	em_token_t *token = em_node_get_token(node, 0);
	em_node_t *right_node = left_node->next;

	em_value_t left = em_context_visit(context, left_node);
	if (!EM_VALUE_OK(left)) return EM_VALUE_FAIL;

	em_value_t right = em_context_visit(context, right_node);
	if (!EM_VALUE_OK(right)) {

		em_value_delete(left);
		return EM_VALUE_FAIL;
	}

	/* operation */
	em_value_t result;

	if (token->type == EM_TOKEN_TYPE_PLUS)
		result = em_value_add(left, right, &node->pos);

	else if (token->type == EM_TOKEN_TYPE_MINUS)
		result = em_value_subtract(left, right, &node->pos);

	else if (token->type == EM_TOKEN_TYPE_ASTERISK)
		result = em_value_multiply(left, right, &node->pos);

	else if (token->type == EM_TOKEN_TYPE_SLASH)
		result = em_value_divide(left, right, &node->pos);

	else if (token->type == EM_TOKEN_TYPE_MODULO)
		result = em_value_modulo(left, right, &node->pos);

	else if (token->type == EM_TOKEN_TYPE_BITWISE_OR)
		result = em_value_or(left, right, &node->pos);

	else if (token->type == EM_TOKEN_TYPE_BITWISE_AND)
		result = em_value_and(left, right, &node->pos);

	else if (token->type == EM_TOKEN_TYPE_BITWISE_LEFT_SHIFT)
		result = em_value_shift_left(left, right, &node->pos);

	else if (token->type == EM_TOKEN_TYPE_BITWISE_RIGHT_SHIFT)
		result = em_value_shift_right(left, right, &node->pos);

	else if (token->type == EM_TOKEN_TYPE_DOUBLE_EQUALS)
		result = em_value_compare_equal(left, right, &node->pos);

	else if (token->type == EM_TOKEN_TYPE_LESS_THAN)
		result = em_value_compare_less_than(left, right, &node->pos);

	else if (token->type == EM_TOKEN_TYPE_LESS_THAN_EQUALS)
		result = EM_VALUE_INT_INV(em_value_compare_greater_than(left, right, &node->pos));

	else if (token->type == EM_TOKEN_TYPE_GREATER_THAN)
		result = em_value_compare_greater_than(left, right, &node->pos);

	else if (token->type == EM_TOKEN_TYPE_GREATER_THAN_EQUALS)
		result = EM_VALUE_INT_INV(em_value_compare_less_than(left, right, &node->pos));

	else if (!strcmp(token->value, "or"))
		result = em_value_compare_or(left, right, &node->pos);

	else if (!strcmp(token->value, "and"))
		result = em_value_compare_and(left, right, &node->pos);

	else {
		em_log_runtime_error(&node->pos, "Unsupported operation ('%s')", token->value);
		result = EM_VALUE_FAIL;
	}

	em_value_delete(left);
	em_value_delete(right);
	return result;
}

/* visit member access */
EM_API em_value_t em_context_visit_access(em_context_t *context, em_node_t *node) {

	em_node_t *container_node = node->first;
	em_token_t *name_token = em_node_get_token(node, 0);
	em_node_t *index_node = container_node->next;

	em_value_t container = em_context_visit(context, container_node);
	if (!EM_VALUE_OK(container)) return EM_VALUE_FAIL;

	em_value_t value = EM_VALUE_FAIL, index = EM_VALUE_FAIL;
	if (index_node) {

		index = em_context_visit(context, index_node);
		if (!EM_VALUE_OK(index)) {

			em_value_delete(container);
			return EM_VALUE_FAIL;
		}
		value = em_value_get_by_index(container, index, &node->pos);

		if (!EM_VALUE_OK(value) && !em_log_catch(NULL))
			em_log_runtime_error(&node->pos, "Invalid index");
	}
	else {

		em_hash_t hash = em_utf8_strhash(name_token->value);
		value = em_value_get_by_hash(container, hash, &node->pos);

		if (!EM_VALUE_OK(value) && !em_log_catch(NULL))
			em_log_runtime_error(&node->pos, "Attribute '%s' not defined", name_token->value);
	}

	em_value_delete(index);
	return value;
}

/* visit call */
EM_API em_value_t em_context_visit_call(em_context_t *context, em_node_t *node) {

	em_node_t *call_node = node->first;

	em_value_t call = em_context_visit(context, call_node);
	if (!EM_VALUE_OK(call)) return EM_VALUE_FAIL;

	em_value_t args[EM_FUNCTION_MAX_ARGUMENTS];
	size_t nargs = 0;

	/* get argument values */
	em_node_t *arg_node = call_node->next;
	while (arg_node && nargs < EM_FUNCTION_MAX_ARGUMENTS) {

		args[nargs] = em_context_visit(context, arg_node);
		if (!EM_VALUE_OK(args[nargs])) {

			for (size_t i = 0; i < nargs; i++)
				em_value_decref(args[i]);
			em_value_delete(call);
			return EM_VALUE_FAIL;
		}
		em_value_incref(args[nargs]);
		arg_node = arg_node->next;
		nargs++;
	}

	em_value_t result = em_value_call(context, call, args, nargs, &node->pos);

	for (size_t i = 0; i < nargs; i++) {

		if (em_value_is(result, args[i]))
			em_value_decref_no_free(args[i]);
		else em_value_decref(args[i]);
	}
	em_value_delete(call);
	return result;
}

/* visit continue statement */
EM_API em_value_t em_context_visit_continue(em_context_t *context, em_node_t *node) {

	em_log_raise("SystemContinue", &node->pos, "Not in a loop");
	return EM_VALUE_FAIL;
}

/* visit break statement */
EM_API em_value_t em_context_visit_break(em_context_t *context, em_node_t *node) {

	em_log_raise("SystemBreak", &node->pos, "Not in a loop");
	return EM_VALUE_FAIL;
}

/* visit return statement */
EM_API em_value_t em_context_visit_return(em_context_t *context, em_node_t *node) {

	em_node_t *value_node = node->first;

	em_value_t value = em_context_visit(context, value_node);
	if (!EM_VALUE_OK(value)) return EM_VALUE_FAIL;

	context->pass = value;

	em_log_raise("SystemReturn", &node->pos, "Not in a function");
	return EM_VALUE_FAIL;
}

/* visit include statement */
EM_API em_value_t em_context_visit_include(em_context_t *context, em_node_t *node) {

	em_node_t *path_node = node->first;

	em_value_t path = em_context_visit(context, path_node);
	if (!EM_VALUE_OK(path)) return EM_VALUE_FAIL;

	if (!em_is_string(path)) {

		em_log_runtime_error(&path_node->pos, "Expected string for path");
		em_value_delete(path);
		return EM_VALUE_FAIL;
	}

	const em_wchar_t *wpath = EM_STRING(EM_OBJECT_FROM_VALUE(path))->data;
	em_wpath_fix(pathbuf2, PATHBUFSZ, wpath);

	em_value_t result = em_context_run_file(context, &node->pos, pathbuf2);
	em_value_delete(path);

	return result;
}

/* visit let statement */
EM_API em_value_t em_context_visit_let(em_context_t *context, em_node_t *node) {

	em_node_t *index_node = node->first;
	em_node_t *value_node = index_node->next;

	if (!value_node) {

		value_node = index_node;
		index_node = NULL;
	}

	em_value_t value = em_context_visit(context, value_node);
	if (!EM_VALUE_OK(value)) return EM_VALUE_FAIL;

	em_value_t index = EM_VALUE_FAIL;
	if (index_node) {

		index = em_context_visit(context, index_node);
		if (!EM_VALUE_OK(index)) {

			em_value_delete(value);
			return EM_VALUE_FAIL;
		}
	}

	/* resolve names up until the last name, or including the last name if an index is provided */
	size_t ntokens = node->tokens.nitems;

	em_value_t container = context->scopestack[context->nscopestack-1];
	const char *prevname = NULL;
	for (size_t i = 0; i < (index_node? ntokens: ntokens-1); i++) {

		em_token_t *token = em_node_get_token(node, i);
		em_hash_t hash = em_utf8_strhash(token->value);

		container = em_value_get_by_hash(container, hash, &token->pos);
		if (!EM_VALUE_OK(container)) {

			if (!em_log_catch(NULL)) {

				if (prevname) em_log_runtime_error(&token->pos, "Attribute '%s' not defined", token->value);
				else em_log_runtime_error(&token->pos, "Variable '%s' not defined", token->value);
			}

			em_value_delete(index);
			em_value_delete(value);
			return EM_VALUE_FAIL;
		}
		prevname = token->value;
	}
	em_token_t *name_token = em_node_get_token(node, ntokens-1);

	/* set value */
	if (index_node) {

		if (em_value_set_by_index(container, index, value, &node->pos) != EM_RESULT_SUCCESS) {

			if (!em_log_catch(NULL))
				em_log_runtime_error(&node->pos, "Invalid index");

			em_value_delete(index);
			em_value_delete(value);
			return EM_VALUE_FAIL;
		}
	}
	else {

		em_hash_t hash = em_utf8_strhash(name_token->value);
		if (em_value_set_by_hash(container, hash, value, &node->pos) != EM_RESULT_SUCCESS) {

			if (!em_log_catch(NULL))
				em_log_runtime_error(&node->pos, "Attribute '%s' not defined", name_token->value);

			em_value_delete(index);
			em_value_delete(value);
			return EM_VALUE_FAIL;
		}
	}

	em_value_delete(index);
	return value;
}

/* visit if statement */
EM_API em_value_t em_context_visit_if(em_context_t *context, em_node_t *node) {

	em_node_t *condition_node = node->first;
	em_node_t *body_node = condition_node->next;

	em_value_t result = em_none;

	em_value_t condition = em_context_visit(context, condition_node);
	if (!EM_VALUE_OK(condition)) return EM_VALUE_FAIL;

	em_inttype_t truthiness = em_value_is_true(condition, &condition_node->pos).value.te_inttype;
	em_value_delete(condition);

	if (truthiness) {

		result = em_context_visit(context, body_node);
		if (!EM_VALUE_OK(result)) return EM_VALUE_FAIL;

		return result;
	}

	/* evaluate elif and else statements */
	condition_node = body_node->next;
	body_node = condition_node? condition_node->next: NULL;

	while (condition_node) {

		/* is else statement */
		if (!body_node) {

			body_node = condition_node;
			condition_node = NULL;
		}

		em_bool_t truthiness = EM_TRUE;
		if (condition_node) {

			em_value_t condition = em_context_visit(context, condition_node);
			if (!EM_VALUE_OK(condition)) {

				em_value_delete(result);
				return EM_VALUE_FAIL;
			}

			truthiness = em_value_is_true(condition, &condition_node->pos).value.te_inttype? EM_TRUE: EM_FALSE;
			em_value_delete(condition);
		}

		/* evaluate body */
		if (truthiness) {

			em_value_delete(result);

			result = em_context_visit(context, body_node);
			if (!EM_VALUE_OK(result))
				return EM_VALUE_FAIL;

			break;
		}

		condition_node = body_node->next;
		body_node = condition_node? condition_node->next: NULL;
	}
	return result;
}

/* visit for statement */
EM_API em_value_t em_context_visit_for(em_context_t *context, em_node_t *node) {

	em_token_t *name_token = em_node_get_token(node, 0);
	em_node_t *start_node = node->first;
	em_node_t *end_node = start_node->next;
	em_node_t *body_node = end_node->next;

	em_value_t start = em_context_visit(context, start_node);
	if (!EM_VALUE_OK(start)) return EM_VALUE_FAIL;

	em_value_t end = em_context_visit(context, end_node);
	if (!EM_VALUE_OK(end)) {

		em_value_delete(start);
		return EM_VALUE_FAIL;
	}

	if (start.type != EM_VALUE_TYPE_INT || end.type != EM_VALUE_TYPE_INT) {

		em_log_runtime_error(&node->pos, "Expected integers for start and end values");
		em_value_delete(start);
		em_value_delete(end);
		return EM_VALUE_FAIL;
	}

	/* evaluate body */
	em_value_t result = em_none;
	em_hash_t hash = em_utf8_strhash(name_token->value);

	for (em_inttype_t i = start.value.te_inttype; i < end.value.te_inttype; i++) {

		em_context_set_value(context, hash, EM_VALUE_INT(i));
		em_value_delete(result);

		result = em_context_visit(context, body_node);
		if (!EM_VALUE_OK(result)) {

			/* continue or break statement */
			if (em_log_catch("SystemContinue")) {

				result = em_none;
				em_log_clear();
				continue;
			}
			if (em_log_catch("SystemBreak")) {

				result = em_none;
				em_log_clear();
				break;
			}
			else return EM_VALUE_FAIL;
		}

		/* update i */
		em_value_t value = em_context_get_value(context, hash);
		if (value.type != EM_VALUE_TYPE_INT) {

			em_log_runtime_error(&node->pos, "Expected integer for iterator");
			em_value_delete(result);
			return EM_VALUE_FAIL;
		}
		i = value.value.te_inttype;
	}
	return result;
}

/* visit foreach statement */
EM_API em_value_t em_context_visit_foreach(em_context_t *context, em_node_t *node) {

	em_token_t *name_token = em_node_get_token(node, 0);
	em_node_t *iterable_node = node->first;
	em_node_t *body_node = iterable_node->next;

	em_value_t iterable = em_context_visit(context, iterable_node);
	if (!EM_VALUE_OK(iterable)) return EM_VALUE_FAIL;

	em_value_t length = em_value_length_of(iterable, &node->pos);
	if (!EM_VALUE_OK(length)) {

		em_value_delete(iterable);
		return EM_VALUE_FAIL;
	}

	/* evaluate body */
	em_value_t result = em_none;
	for (em_inttype_t i = 0; i < length.value.te_inttype; i++) {

		em_value_t value = em_value_get_by_index(iterable, EM_VALUE_INT(i), &node->pos);
		em_value_delete(result);

		if (!EM_VALUE_OK(value)) {

			if (!em_log_catch(NULL))
				em_log_runtime_error(&node->pos, "Couldn't finish iteration");
			em_value_delete(iterable);
			return EM_VALUE_FAIL;
		}
		em_context_set_value(context, em_utf8_strhash(name_token->value), value);

		result = em_context_visit(context, body_node);
		if (!EM_VALUE_OK(result)) {

			/* continue or break statement */
			if (em_log_catch("SystemContinue")) {

				result = em_none;
				em_log_clear();
				continue;
			}
			if (em_log_catch("SystemBreak")) {

				result = em_none;
				em_log_clear();
				break;
			}
			else {

				em_value_delete(iterable);
				return EM_VALUE_FAIL;
			}
		}
	}
	em_value_delete(iterable);
	return result;
}

/* visit while statement */
EM_API em_value_t em_context_visit_while(em_context_t *context, em_node_t *node) {

	em_node_t *condition_node = node->first;
	em_node_t *body_node = condition_node->next;

	em_value_t condition = em_context_visit(context, condition_node);
	if (!EM_VALUE_OK(condition)) return EM_VALUE_FAIL;

	em_inttype_t truthiness = em_value_is_true(condition, &node->pos).value.te_inttype;
	em_value_delete(condition);

	em_value_t result = em_none;
	while (truthiness) {

		em_value_delete(result);

		result = em_context_visit(context, body_node);
		if (!EM_VALUE_OK(result)) {

			/* continue or break statement */
			if (em_log_catch("SystemContinue")) {

				result = em_none;
				em_log_clear();
				continue;
			}
			if (em_log_catch("SystemBreak")) {

				result = em_none;
				em_log_clear();
				break;
			}
			else return EM_VALUE_FAIL;
		}

		condition = em_context_visit(context, condition_node);
		if (!EM_VALUE_OK(condition)) {

			em_value_delete(result);
			return EM_VALUE_FAIL;
		}

		truthiness = em_value_is_true(condition, &node->pos).value.te_inttype;
		em_value_delete(condition);
	}
	return result;
}

/* visit func statement */
EM_API em_value_t em_context_visit_func(em_context_t *context, em_node_t *node) {

	size_t firstarg = 0;
	const char *name = "<anonymous>";
	if (node->flags) name = em_node_get_token(node, firstarg++)->value;

	em_node_t *body_node = node->first;

	/* collect arguments */
	size_t nargnames = 0;
	const char *argnames[EM_FUNCTION_MAX_ARGUMENTS];

	while (nargnames < EM_FUNCTION_MAX_ARGUMENTS) {

		em_token_t *token = em_node_get_token(node, firstarg + nargnames);
		if (!token) break;

		argnames[nargnames++] = token->value;
	}

	/* set value */
	em_value_t value = em_function_new(node, body_node, name, nargnames, argnames);
	if (node->flags) em_context_set_value(context, em_utf8_strhash(name), value);

	return value;
}

/* visit class statement */
EM_API em_value_t em_context_visit_class(em_context_t *context, em_node_t *node) {

	em_token_t *name_token = em_node_get_token(node, 0);
	em_node_t *base_node = node->first;
	em_node_t *body_node = base_node->next;

	if (!body_node) {

		body_node = base_node;
		base_node = NULL;
	}

	/* evaluate base class */
	em_value_t base = EM_VALUE_FAIL;
	if (base_node) {

		base = em_context_visit(context, base_node);
		if (!EM_VALUE_OK(base)) return EM_VALUE_FAIL;

		if (!em_is_class(base)) {

			em_log_runtime_error(&base_node->pos, "Base class is not a class");
			em_value_delete(base);
			return EM_VALUE_FAIL;
		}
	}

	/* evaluate body of class */
	if (em_context_push_scope(context) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	em_value_t result = em_context_visit(context, body_node);
	if (!EM_VALUE_OK(result)) {

		em_context_pop_scope(context);
		em_value_delete(base);
		return EM_VALUE_FAIL;
	}

	em_value_t class = em_class_new(node, name_token->value, base, context->scopestack[context->nscopestack-1]);
	em_context_pop_scope(context);

	/* set value */
	em_context_set_value(context, em_utf8_strhash(name_token->value), class);
	return class;
}

/* visit puts statement */
EM_API em_value_t em_context_visit_puts(em_context_t *context, em_node_t *node) {

	em_node_t *cur = node->first;

	em_value_t result = em_none;
	while (cur) {

		em_value_delete(result);

		result = em_context_visit(context, cur);
		if (!EM_VALUE_OK(result)) return EM_VALUE_FAIL;

		em_value_t string = em_value_to_string(result, &node->pos);
		if (!EM_VALUE_OK(string)) {

			em_value_delete(result);
			return EM_VALUE_FAIL;
		}

		em_string_t *strobject = EM_STRING(EM_OBJECT_FROM_VALUE(string));
		em_wchar_write(stdout, strobject->data, strobject->length);
		if (cur->next) fprintf(stdout, " ");

		if (result.type != string.type || result.value.t_voidp != string.value.t_voidp)
			em_value_delete(string);
		cur = cur->next;
	}
	fprintf(stdout, "\n");
	return result;
}

/* destroy context */
EM_API void em_context_destroy(em_context_t *context) {

	if (!context || !context->init) return;

	for (size_t i = 0; i < context->nscopestack; i++)
		em_value_decref(context->scopestack[i]);

	em_recfile_t *recfile = context->rec_first;
	while (recfile) {

		em_recfile_t *next = recfile->next;
		em_free(recfile);
		recfile = next;
	}

	em_parser_destroy(&context->parser);
	em_lexer_destroy(&context->lexer);
	context->init = EM_FALSE;
}

/* free context */
EM_API void em_context_free(em_context_t *context) {

	if (!context) return;

	em_context_destroy(context);
	free(context);
}
