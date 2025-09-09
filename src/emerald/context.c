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
	[EM_NODE_TYPE_LET] = em_context_visit_let,
};

/* NOTE: Always leave pathbuf1 for reuse, even if used previously */
#define PATHBUFSZ 4096
static char pathbuf1[PATHBUFSZ];
static char pathbuf2[PATHBUFSZ];

/* create context */
EM_API em_context_t *em_context_new(void) {

	em_context_t *context = em_malloc(sizeof(em_context_t));
	if (!context) return NULL;

	if (em_context_init(context) != EM_RESULT_SUCCESS) {

		em_context_free(context);
		return NULL;
	}

	return context;
}

/* initialize context */
EM_API em_result_t em_context_init(em_context_t *context) {

	if (!context) return EM_RESULT_FAILURE;
	else if (context->init) {

		em_log_fatal("Context already initialized");
		return EM_RESULT_FAILURE;
	}

	context->lexer = EM_LEXER_INIT;
	context->parser = EM_PARSER_INIT;

	if (em_lexer_init(&context->lexer) != EM_RESULT_SUCCESS)
		return EM_RESULT_FAILURE;
	if (em_parser_init(&context->parser) != EM_RESULT_SUCCESS)
		return EM_RESULT_FAILURE;

	/* set up initial directory stack */
	context->ndirstack = 1;
	context->dirstack[0] = ".";
#ifndef DEBUG
	context->ndirstack++;
	context->dirstack[1] = EM_STDLIB_DIR;
#endif
	context->nscopestack = 1;
	context->scopestack[0] = em_map_new();
	em_value_incref(context->scopestack[0]);
	context->rec_first = NULL;
	context->rec_last = NULL;

	context->init = EM_TRUE;
	return EM_RESULT_SUCCESS;
}

/* run code */
EM_API em_value_t em_context_run_text(em_context_t *context, const char *path, const char *text, em_ssize_t len) {

	if (!context || !context->init) return EM_VALUE_FAIL;

	em_lexer_reset(&context->lexer, path, text, len);
	if (em_lexer_make_tokens(&context->lexer) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	em_parser_reset(&context->parser, context->lexer.first);
	if (em_parser_parse(&context->parser) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	return em_context_visit(context, context->parser.node);
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
			return EM_VALUE_INT(0);
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
	em_value_t res = em_context_run_text(context, recfile->rpath, fbuf, (em_ssize_t)len);

	/* clean up */
	em_free(fbuf);
	if (buf) {

		(void)em_context_popdir(context);
		em_free(buf);
	}

	/* add file to run list */
	if (EM_VALUE_OK(res)) {

		if (!context->rec_first) context->rec_first = recfile;
		if (context->rec_last) context->rec_last->next = recfile;
		context->rec_last = recfile;
	}
	else em_free(recfile);
	
	return res;
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
	em_value_t value = EM_VALUE_INT(0);
	while (cur) {

		value = em_context_visit(context, cur);
		if (!EM_VALUE_OK(value)) return EM_VALUE_FAIL;

		cur = cur->next;
		if (cur) em_value_delete(value);
	}
	return value;
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
	return em_string_new_from_utf8(token->value, strlen(token->value));
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

	if (!strcmp(token->value, "+"))
		result = em_value_add(left, right, &node->pos);
	else if (!strcmp(token->value, "-"))
		result = em_value_subtract(left, right, &node->pos);
	else if (!strcmp(token->value, "*"))
		result = em_value_multiply(left, right, &node->pos);
	else if (!strcmp(token->value, "/"))
		result = em_value_divide(left, right, &node->pos);
	else if (!strcmp(token->value, "|"))
		result = em_value_or(left, right, &node->pos);
	else if (!strcmp(token->value, "&"))
		result = em_value_and(left, right, &node->pos);
	else if (!strcmp(token->value, "<<"))
		result = em_value_shift_left(left, right, &node->pos);
	else if (!strcmp(token->value, ">>"))
		result = em_value_shift_right(left, right, &node->pos);
	else if (!strcmp(token->value, "=="))
		result = em_value_compare_equal(left, right, &node->pos);
	else if (!strcmp(token->value, "<"))
		result = em_value_compare_less_than(left, right, &node->pos);
	else if (!strcmp(token->value, "<="))
		result = EM_VALUE_INT_INV(em_value_compare_greater_than(left, right, &node->pos));
	else if (!strcmp(token->value, ">"))
		result = em_value_compare_greater_than(left, right, &node->pos);
	else if (!strcmp(token->value, ">="))
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
