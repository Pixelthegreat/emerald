/*
 * Copyright 2025-2026, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <emerald/core.h>
#include <emerald/context.h>
#include <emerald/none.h>
#include <emerald/utf8.h>
#include <emerald/wchar.h>
#include <emerald/hash.h>
#include <emerald/path.h>
#include <emerald/string.h>
#include <emerald/list.h>
#include <emerald/map.h>
#include <emerald/function.h>
#include <emerald/class.h>
#include <emerald/bytecode.h>

/* operation names */
static const char *op_names[EM_CODE_OP_COUNT] = {
	NULL,
	"PCINT", "PCFLT", "PCSTR",
	"PTRUE", "PFLSE", "PNONE", "POP",
	"CLIST", "CMAP",
	"UNEG", "UNOT", "UBNOT",
	"UINC", "UDEC",
	"BADD", "BSUB", "BMUL",
	"BDIV", "BMOD", "BBOR",
	"BBXOR", "BBAND", "BBLSH",
	"BBRSH", "BEQ", "BNEQ",
	"BLT", "BGT",
	"LOAD", "LDNM", "LDIDX",
	"STOR", "STNM", "STIDX",
	"JMP", "JTR", "JNTR", "JPNTR",
	"CALL", "SAVE1", "SAVE3",
	"RSTR1", "RSTR2", "RSTR3",
	"DSCD1", "DSCD3",
	"DCLS", "DFUNC", "DBGN",
	"ESETL", "ESETC", "PUTS",
	"INCLUDE", "BLTJXPIPI",
	"LEN", "R1EISNTP",
};

/* create code object with node */
EM_API em_code_t *em_code_new_node(em_node_t *node, const char *path) {

	size_t len = strlen(path);
	size_t size = sizeof(em_code_t) + len + 1;

	em_code_t *code = EM_CODE(em_refobj_new(&em_reflist_code, size, EM_CLEANUP_MODE_IMMEDIATE));
	if (!code) return NULL;

EM_CODE_INCREF(code);

	code->type = EM_CODE_TYPE_TREE;
	code->tree = node;

	memcpy(code->path, path, len);
	code->path[len] = 0;

	return code;
}

/* create code object with bytecode slice */
EM_API em_code_t *em_code_new_binary(em_code_slice_t binary, const char *path) {

	size_t len = strlen(path);
	size_t size = sizeof(em_code_t) + len + 1;

	em_code_t *code = EM_CODE(em_refobj_new(&em_reflist_code, size, EM_CLEANUP_MODE_IMMEDIATE));
	if (!code) return NULL;

	EM_CODE_INCREF(code);

	code->type = EM_CODE_TYPE_BINARY;
	code->binary = binary;

	memcpy(code->path, path, len);
	code->path[len] = 0;

	return code;
}

/* run code */
EM_API em_value_t em_code_run(em_code_t *code, em_context_t *context) {

	em_pos_t pos = {0};

	switch (code->type) {
		case EM_CODE_TYPE_TREE:
			return em_context_visit(context, code->tree);
	}
	return EM_VALUE_FAIL;
}

/* write uint8 value */
EM_API void em_code_write_uint8(em_code_slice_t *slice, uint8_t value) {

	if (slice->position+1 > slice->length)
		return;
	*((uint8_t *)(slice->data + slice->position++)) = value;
}

/* write uint16 value */
EM_API void em_code_write_uint16(em_code_slice_t *slice, uint16_t value) {

	if (slice->position+2 > slice->length)
		return;
	*((uint16_t *)(slice->data + slice->position)) = value;
	slice->position += 2;
}

/* write uint32 value */
EM_API void em_code_write_uint32(em_code_slice_t *slice, uint32_t value) {

	if (slice->position+4 > slice->length)
		return;
	*((uint32_t *)(slice->data + slice->position)) = value;
	slice->position += 4;
}

/* write uint64 value */
EM_API void em_code_write_uint64(em_code_slice_t *slice, uint64_t value) {

	if (slice->position+8 > slice->length)
		return;
	*((uint64_t *)(slice->data + slice->position)) = value;
	slice->position += 8;
}

/* write int8 value */
EM_API void em_code_write_int8(em_code_slice_t *slice, int8_t value) {

	em_code_write_uint8(slice, (uint8_t)value);
}

/* write int16 value */
EM_API void em_code_write_int16(em_code_slice_t *slice, int16_t value) {

	em_code_write_uint16(slice, (uint16_t)value);
}

/* write int32 value */
EM_API void em_code_write_int32(em_code_slice_t *slice, int32_t value) {

	em_code_write_uint32(slice, (uint32_t)value);
}

/* write int64 value */
EM_API void em_code_write_int64(em_code_slice_t *slice, int64_t value) {

	em_code_write_uint64(slice, (uint64_t)value);
}

/* write emerald int value */
EM_API void em_code_write_inttype(em_code_slice_t *slice, em_inttype_t value) {

	switch (sizeof(value)) {
		case 4: em_code_write_uint32(slice, *(uint32_t *)&value); break;
		case 8: em_code_write_uint64(slice, *(uint64_t *)&value); break;
	}
}

/* write float value */
EM_API void em_code_write_float(em_code_slice_t *slice, float value) {

	if (slice->position+4 > slice->length)
		return;
	*((float *)(slice->data + slice->position)) = value;
	slice->position += 4;
}

/* write double value */
EM_API void em_code_write_double(em_code_slice_t *slice, double value) {

	if (slice->position+8 > slice->length)
		return;
	*((double *)(slice->data + slice->position)) = value;
	slice->position += 8;
}

/* write emerald float value */
EM_API void em_code_write_floattype(em_code_slice_t *slice, em_floattype_t value) {

	switch (sizeof(value)) {
		case 4: em_code_write_float(slice, value); break;
		case 8: em_code_write_double(slice, value); break;
	}
}

/* write string */
EM_API void em_code_write_string(em_code_slice_t *slice, const char *value, size_t len) {

	em_code_write_uint16(slice, (uint16_t)len);
	for (size_t i = 0; i < len; i++)
		em_code_write_uint8(slice, (uint8_t)value[i]);
	em_code_write_uint8(slice, 0);
}

/* write hashed string */
EM_API void em_code_write_hashed_string(em_code_slice_t *slice, const char *value, size_t len, em_hash_t hash) {

	em_code_write_uint32(slice, hash);
	em_code_write_string(slice, value, len);
}

/* read uint8 value */
EM_API uint8_t em_code_read_uint8(em_code_slice_t *slice) {

	if (slice->position+1 > slice->length)
		return 0;
	return *(uint8_t *)(slice->data + slice->position++);
}

/* read uint16 value */
EM_API uint16_t em_code_read_uint16(em_code_slice_t *slice) {

	if (slice->position+2 > slice->length)
		return 0;

	uint16_t value = *(uint16_t *)(slice->data + slice->position);
	slice->position += 2;

	return value;
}

/* read uint32 value */
EM_API uint32_t em_code_read_uint32(em_code_slice_t *slice) {

	if (slice->position+4 > slice->length)
		return 0;

	uint32_t value = *(uint32_t *)(slice->data + slice->position);
	slice->position += 4;

	return value;
}

/* read uint64 value */
EM_API uint64_t em_code_read_uint64(em_code_slice_t *slice) {

	if (slice->position+8 > slice->length)
		return 0;

	uint64_t value = *(uint64_t *)(slice->data + slice->position);
	slice->position += 8;

	return value;
}

/* read int8 value */
EM_API int8_t em_code_read_int8(em_code_slice_t *slice) {

	return (int8_t)em_code_read_uint8(slice);
}

/* read int16 value */
EM_API int16_t em_code_read_int16(em_code_slice_t *slice) {

	return (int16_t)em_code_read_uint16(slice);
}

/* read int32 value */
EM_API int32_t em_code_read_int32(em_code_slice_t *slice) {

	return (int32_t)em_code_read_uint32(slice);
}

/* read int64 value */
EM_API int64_t em_code_read_int64(em_code_slice_t *slice) {

	return (int64_t)em_code_read_uint64(slice);
}

/* read emerald int value */
EM_API em_inttype_t em_code_read_inttype(em_code_slice_t *slice) {

	switch (sizeof(em_inttype_t)) {
		case 1: return em_code_read_uint8(slice);
		case 2: return em_code_read_uint16(slice);
		case 4: return em_code_read_uint32(slice);
		case 8: return em_code_read_uint64(slice);
	}
	return 0;
}

/* read float value */
EM_API float em_code_read_float(em_code_slice_t *slice) {

	if (slice->position+4 > slice->length)
		return 0;

	float value = *(float *)(slice->data + slice->position);
	slice->position += 4;

	return value;
}

/* read double value */
EM_API double em_code_read_double(em_code_slice_t *slice) {

	if (slice->position+8 > slice->length)
		return 0;

	double value = *(double *)(slice->data + slice->position);
	slice->position += 8;

	return value;
}

/* read emerald float value */
EM_API em_floattype_t em_code_read_floattype(em_code_slice_t *slice) {

	switch (sizeof(em_floattype_t)) {
		case 4: return em_code_read_float(slice);
		case 8: return em_code_read_double(slice);
	}
	return 0;
}

/* read string */
EM_API const char *em_code_read_string(em_code_slice_t *slice) {

	uint16_t len = em_code_read_uint16(slice);
	const char *str = (const char *)(slice->data + slice->position);

	slice->position += (size_t)len + 1;
	return str;
}

/* read string with hash */
EM_API const char *em_code_read_hashed_string(em_code_slice_t *slice, em_hash_t *hash) {

	*hash = em_code_read_uint32(slice);
	return em_code_read_string(slice);
}

/* synchronize position information */
static void set_position_size(em_code_compiler_t *compiler, em_node_t *node, size_t *p_size) {

	if (compiler->pos.line != node->pos.line) {

		compiler->pos.line = node->pos.line;
		*p_size += 3; /* ESETL, uint16 */
	}
	if (compiler->pos.column != node->pos.column) {

		compiler->pos.column = node->pos.column;
		*p_size += 2; /* ESETC, uint8 */
	}
}

static void set_position(em_code_compiler_t *compiler, em_node_t *node) {

	em_code_slice_t *slice = compiler->slice;
	if (compiler->pos.line != node->pos.line) {

		compiler->pos.line = node->pos.line;
		em_code_write_uint8(slice, EM_CODE_OP_ESETL);
		em_code_write_uint16(slice, (uint16_t)node->pos.line);
	}
	if (compiler->pos.column != node->pos.column) {

		compiler->pos.column = node->pos.column;
		em_code_write_uint8(slice, EM_CODE_OP_ESETC);
		em_code_write_uint8(slice, (uint8_t)node->pos.column);
	}
}

/* predict final size of node (useful for branches) */
#define STR_SIZE(len) ((len)+3)
#define HASH_STR_SIZE(len) (STR_SIZE(len)+4)
#define PC_REL(p_slice, p_pos) ((int32_t)(p_pos) - (int32_t)((p_slice)->position) - 4)

EM_API size_t em_code_get_size(em_code_compiler_t *compiler, em_node_t *node) {

	em_token_t *token;

	size_t size = 0;
	em_node_t *orig = node;
	size_t count = 0;

	switch (node->type) {

		/* block */
		case EM_NODE_TYPE_BLOCK:
			for (node = node->first; node; node = node->next) {

				size += em_code_get_size(compiler, node);
				if (node->next) size += 1; /* POP */
			}
			node = orig;
			if (!node->first) size += 1; /* PNONE */
			break;

		/* constants */
		case EM_NODE_TYPE_INT:
			set_position_size(compiler, node, &size);
			size += 1; /* PCINT */
			size += sizeof(em_inttype_t);
			break;
		case EM_NODE_TYPE_FLOAT:
			set_position_size(compiler, node, &size);
			size += 1; /* PCFLT */
			size += sizeof(em_floattype_t);
			break;
		case EM_NODE_TYPE_STRING:
			set_position_size(compiler, node, &size);
			size += 1; /* PCSTR */
			size += STR_SIZE(em_node_get_token(node, 0)->length);
			break;

		/* load variable */
		case EM_NODE_TYPE_IDENTIFIER:
			set_position_size(compiler, node, &size);
			size += 1; /* LOAD */
			size += HASH_STR_SIZE(em_node_get_token(node, 0)->length);
			break;

		/* construct list or map, function call or puts */
		case EM_NODE_TYPE_LIST:
		case EM_NODE_TYPE_MAP:
		case EM_NODE_TYPE_CALL:
		case EM_NODE_TYPE_PUTS:
			for (node = node->first; node; node = node->next)
				size += em_code_get_size(compiler, node);
			node = orig;
			set_position_size(compiler, node, &size);
			size += 3; /* op, uint16 */
			break;

		/* unary and binary operations */
		case EM_NODE_TYPE_UNARY_OPERATION:
			size += em_code_get_size(compiler, node->first);
			set_position_size(compiler, node, &size);
			if (em_node_get_token(node, 0)->type != EM_TOKEN_TYPE_PLUS)
				size += 1; /* op */
			break;
		case EM_NODE_TYPE_BINARY_OPERATION:
			token = em_node_get_token(node, 0);

			if (strchr("ao", *token->value)) {

				size += em_code_get_size(compiler, node->first);
				size += 5; /* JTR / JNTR */
				size += 1; /* POP */
				size += em_code_get_size(compiler, node->first->next);
				break;
			}
			size += em_code_get_size(compiler, node->first);
			size += em_code_get_size(compiler, node->first->next);
			set_position_size(compiler, node, &size);
			size += 1; /* op */
			if (token->type == EM_TOKEN_TYPE_LESS_THAN_EQUALS ||
			    token->type == EM_TOKEN_TYPE_GREATER_THAN_EQUALS)
				size += 1; /* UNOT */
			break;

		/* member access */
		case EM_NODE_TYPE_ACCESS:
			if (node->first->next) { /* indexed */

				size += em_code_get_size(compiler, node->first);
				size += em_code_get_size(compiler, node->first->next);
				set_position_size(compiler, node, &size);
				size += 1; /* LDIDX */
			}
			else { /* named */
				size += em_code_get_size(compiler, node->first);
				set_position_size(compiler, node, &size);
				size += 1; /* LDNM */
				size += HASH_STR_SIZE(em_node_get_token(node, 0)->length);
			}
			break;

		/* continue and break */
		case EM_NODE_TYPE_CONTINUE:
		case EM_NODE_TYPE_BREAK:
			set_position_size(compiler, node, &size);
			size += 1; /* PTRUE / PFLSE */
			size += 1; /* RSTR3 */
			break;

		/* return, raise and include */
		case EM_NODE_TYPE_RETURN:
		case EM_NODE_TYPE_RAISE:
		case EM_NODE_TYPE_INCLUDE:
			size += em_code_get_size(compiler, node->first);
			set_position_size(compiler, node, &size);
			size += 1; /* RSTR2 / RSTR1 / INCLUDE */
			break;

		/* let statement */
		case EM_NODE_TYPE_LET:
			if (node->first->next) { /* indexed */

				set_position_size(compiler, node, &size);
				for (size_t i = 0; i < node->tokens.nitems; i++) {

					size += 1; /* LOAD / LDNM */
					size += HASH_STR_SIZE(
						em_node_get_token(node, i)->length
					);
				}
				size += em_code_get_size(compiler, node->first);
				size += em_code_get_size(compiler, node->first->next);
				set_position_size(compiler, node, &size);
				size += 1; /* STIDX */
			}
			else { /* named */
				set_position_size(compiler, node, &size);
				for (size_t i = 0; i < node->tokens.nitems-1; i++) {

					size += 1; /* LOAD / LDNM */
					size += HASH_STR_SIZE(
						em_node_get_token(node, i)->length
					);
				}
				size += em_code_get_size(compiler, node->first);
				set_position_size(compiler, node, &size);
				size += 1; /* STNM / STOR */
				size += HASH_STR_SIZE(
					em_node_get_token(node, node->tokens.nitems-1)->length
				);
			}
			break;

		/* if statement */
		case EM_NODE_TYPE_IF:
			node = node->first;
			while (node) {

				em_node_t *condition_node = node;
				em_node_t *body_node = condition_node->next;

				if (!body_node) {

					body_node = condition_node;
					condition_node = NULL;
				}
				if (condition_node) {

					size += em_code_get_size(compiler, condition_node);
					size += 5; /* JMP / JNTR, uint32 */
				}

				size += em_code_get_size(compiler, body_node);
				if (body_node->next) size += 5; /* JMP, uint32 */

				node = body_node->next;
			}
			break;

		/* for statement */
		case EM_NODE_TYPE_FOR:
			token = em_node_get_token(node, 0);
			/* @init */
			size += 5; /* SAVE3, @break */
			size += 1; /* PNONE (value produced by @body) */
			size += em_code_get_size(compiler, node->first);
			size += 1; /* STOR */
			size += HASH_STR_SIZE(token->length);
			size += 5; /* JMP, @cond */
			/* @break */
			size += 5; /* JPNTR, @end+1 */
			size += 5; /* SAVE3, @break */
			size += 1; /* PNONE */
			/* @start */
			size += 1; /* LOAD */
			size += HASH_STR_SIZE(token->length);
			size += 1; /* UINC */
			size += 1; /* STOR */
			size += HASH_STR_SIZE(token->length);
			/* @cond */
			size += em_code_get_size(compiler, node->first->next);
			size += 1; /* BLT */
			size += 5; /* JNTR, @end */
			size += 1; /* POP (value produced by @body) */
			/* @body */
			size += em_code_get_size(compiler, node->first->next->next);
			size += 5; /* JMP, @start */
			/* @end */
			size += 1; /* DSCD3 */
			break;

		/* foreach statement */
		case EM_NODE_TYPE_FOREACH:
			token = em_node_get_token(node, 0);
			/* @init */
			size += em_code_get_size(compiler, node->first);
			size += 1; /* LEN */
			size += 1; /* PFLSE */
			size += 1; /* PNONE */
			size += 5; /* SAVE3, @break */
			size += 5; /* JMP, @start */
			/* @break */
			size += 5; /* JNTR, @breakend */
			size += 5; /* SAVE3, @break */
			size += 1; /* PNONE */
			/* @start */
			size += 5; /* BLTJXPIPI, @end */
			size += 1; /* STOR */
			size += HASH_STR_SIZE(token->length);
			size += 1; /* POP */
			/* @body */
			size += em_code_get_size(compiler, node->first->next);
			size += 5; /* JMP, @start */
			/* @breakend */
			size += 4; /* POP, POP, POP, PNONE */
			size += 5; /* JMP, 1 */
			/* @end */
			size += 1; /* DSCD3 */
			break;

		/* while statement */
		case EM_NODE_TYPE_WHILE:
			/* @init */
			size += 1; /* PNONE */
			size += 5; /* SAVE3, @break */
			size += 5; /* JMP, @start */
			/* @break */
			size += 5; /* JPNTR, @end+1 */
			size += 5; /* SAVE3, @break */
			size += 1; /* PNONE */
			/* @start */
			size += em_code_get_size(compiler, node->first);
			size += 5; /* JNTR, @end */
			size += 1; /* POP */
			/* @body */
			size += em_code_get_size(compiler, node->first->next);
			size += 5; /* JMP, @start */
			/* @end */
			size += 1; /* DSCD3 */
			break;

		/* func statement */
		case EM_NODE_TYPE_FUNC:
			size += 2; /* DFUNC, uint8 */
			if (node->flags) {

				token = em_node_get_token(node, 0);
				size += HASH_STR_SIZE(token->length);
			}
			else size += HASH_STR_SIZE(11); /* '<anonymous>' */
			for (size_t i = 0; i < node->tokens.nitems; i++) {

				if (!i && node->flags)
					continue;
				token = em_node_get_token(node, i);
				size += HASH_STR_SIZE(token->length);
			}
			size += 4; /* uint32 */
			size += em_code_get_size(compiler, node->first);

			if (node->flags) {

				token = em_node_get_token(node, 0);
				size += 1; /* STOR */
				size += HASH_STR_SIZE(token->length);
			}
			break;

		/* class statement */
		case EM_NODE_TYPE_CLASS:
			token = em_node_get_token(node, 0);
			if (node->first->next) { /* with base class */

				size += em_code_get_size(compiler, node->first);
				size += 1; /* DBGN */
				size += em_code_get_size(compiler, node->first->next);
			}
			else { /* without base class */
				size += 1; /* PNONE */
				size += 1; /* DBGN */
				size += em_code_get_size(compiler, node->first);
			}
			size += 1; /* DCLS */
			size += HASH_STR_SIZE(token->length);
			break;

		/* try statement */
		case EM_NODE_TYPE_TRY:
			token = em_node_get_token(node, 0);

			/* @init */
			size += 5; /* SAVE1, @catch */
			/* @try */
			size += em_code_get_size(compiler, node->first);
			size += 5; /* JMP, @end */
			/* @catch */
			size += em_code_get_size(compiler, node->first->next);
			size += 1; /* R1EISNTP */
			size += 1; /* STOR */
			size += HASH_STR_SIZE(token->length);
			size += 1; /* POP */
			size += em_code_get_size(compiler, node->first->next->next);
			size += 5; /* JMP, @end+1 */
			/* @end */
			size += 1; /* DSCD1 */
			break;
	}
	orig->code_size = size;
	return size;
}

/* write node */
EM_API void em_code_write(em_code_compiler_t *compiler, em_node_t *node) {

	em_code_slice_t *slice = compiler->slice;
	em_token_t *token;
	em_hash_t hash;
	em_inttype_t it_value;
	em_floattype_t ft_value;
	const char *string;
	em_code_op_t op = 0;
	size_t count = 0;
	size_t pos_a, pos_b, pos_c, pos_d, pos_e, pos_f;

	em_node_t *orig = node;

	switch (node->type) {

		/* block */
		case EM_NODE_TYPE_BLOCK:
			for (node = node->first; node; node = node->next) {

				em_code_write(compiler, node);
				if (node->next) em_code_write_uint8(slice, EM_CODE_OP_POP);
			}
			node = orig;
			if (!node->first) em_code_write_uint8(slice, EM_CODE_OP_PNONE);
			break;

		/* constants */
		case EM_NODE_TYPE_INT:
			set_position(compiler, node);

			token = em_node_get_token(node, 0);
			it_value = 0;
			string = token->value;
			for (; *string >= '0' && *string <= '9'; string++)
				it_value = (it_value * 10) + (em_inttype_t)(*string - '0');

			em_code_write_uint8(slice, EM_CODE_OP_PCINT);
			em_code_write_inttype(slice, it_value);
			break;
		case EM_NODE_TYPE_FLOAT:
			set_position(compiler, node);
			ft_value = 0;
#ifndef _ECLAIR
			token = em_node_get_token(node, 0);
			sscanf(token->value, EM_FLOATTYPE_FORMAT, &ft_value);
#endif
			em_code_write_uint8(slice, EM_CODE_OP_PCFLT);
			em_code_write_floattype(slice, ft_value);
			break;
		case EM_NODE_TYPE_STRING:
			set_position(compiler, node);

			token = em_node_get_token(node, 0);
			em_code_write_uint8(slice, EM_CODE_OP_PCSTR);
			em_code_write_string(slice, token->value, token->length);
			break;

		/* load variable */
		case EM_NODE_TYPE_IDENTIFIER:
			set_position(compiler, node);

			token = em_node_get_token(node, 0);
			hash = em_node_get_value(node, 0).v.te_hash;

			em_code_write_uint8(slice, EM_CODE_OP_LOAD);
			em_code_write_hashed_string(
					slice, token->value,
					token->length, hash
			);
			break;

		/* construct list or puts statement */
		case EM_NODE_TYPE_LIST:
		case EM_NODE_TYPE_PUTS:
			for (node = node->first; node; node = node->next) {

				em_code_write(compiler, node);
				count++;
			}
			node = orig;
			set_position(compiler, node);

			op = EM_CODE_OP_CLIST;
			if (node->type == EM_NODE_TYPE_PUTS)
				op = EM_CODE_OP_PUTS;

			em_code_write_uint8(slice, (uint8_t)op);
			em_code_write_uint16(slice, (uint16_t)count);
			break;

		/* construct map */
		case EM_NODE_TYPE_MAP:
			for (node = node->first; node; node = node->next) {

				em_code_write(compiler, node);
				count++;
			}
			node = orig;
			set_position(compiler, node);

			em_code_write_uint8(slice, EM_CODE_OP_CMAP);
			em_code_write_uint16(slice, (uint16_t)count >> 1);
			break;

		/* unary operation */
		case EM_NODE_TYPE_UNARY_OPERATION:
			em_code_write(compiler, node->first);
			set_position(compiler, node);

			token = em_node_get_token(node, 0);
			if (token->type == EM_TOKEN_TYPE_MINUS)
				em_code_write_uint8(slice, EM_CODE_OP_UNEG);
			else if (token->type == EM_TOKEN_TYPE_BITWISE_NOT)
				em_code_write_uint8(slice, EM_CODE_OP_UBNOT);
			else if (!strcmp(token->value, "not"))
				em_code_write_uint8(slice, EM_CODE_OP_UNOT);
			break;

		/* binary operation */
		case EM_NODE_TYPE_BINARY_OPERATION:
			token = em_node_get_token(node, 0);

			/* short-circuited operations */
			if (strchr("ao", *token->value)) {

				switch (*token->value) {
					case 'a': op = EM_CODE_OP_JNTR; break;
					case 'o': op = EM_CODE_OP_JTR; break;
				}

				em_code_write(compiler, node->first);
				em_code_write_uint8(slice, (uint8_t)op);
				em_code_write_int32(
					slice, (int32_t)node->first->next->code_size+1
				);
				em_code_write_uint8(slice, EM_CODE_OP_POP);
				em_code_write(compiler, node->first->next);
				break;
			}

			/* normal operations */
			em_code_write(compiler, node->first);
			em_code_write(compiler, node->first->next);
			set_position(compiler, node);

			if (token->type == EM_TOKEN_TYPE_LESS_THAN_EQUALS) {

				em_code_write_uint8(slice, EM_CODE_OP_BGT);
				em_code_write_uint8(slice, EM_CODE_OP_UNOT);
				break;
			}
			else if (token->type == EM_TOKEN_TYPE_GREATER_THAN_EQUALS) {

				em_code_write_uint8(slice, EM_CODE_OP_BLT);
				em_code_write_uint8(slice, EM_CODE_OP_UNOT);
				break;
			}

			switch (token->type) {
				case EM_TOKEN_TYPE_PLUS: op = EM_CODE_OP_BADD; break;
				case EM_TOKEN_TYPE_MINUS: op = EM_CODE_OP_BSUB; break;
				case EM_TOKEN_TYPE_ASTERISK: op = EM_CODE_OP_BMUL; break;
				case EM_TOKEN_TYPE_SLASH: op = EM_CODE_OP_BDIV; break;
				case EM_TOKEN_TYPE_MODULO: op = EM_CODE_OP_BMOD; break;
				case EM_TOKEN_TYPE_BITWISE_OR: op = EM_CODE_OP_BBOR; break;
				case EM_TOKEN_TYPE_BITWISE_XOR: op = EM_CODE_OP_BBXOR; break;
				case EM_TOKEN_TYPE_BITWISE_AND: op = EM_CODE_OP_BBAND; break;
				case EM_TOKEN_TYPE_BITWISE_LEFT_SHIFT: op = EM_CODE_OP_BBLSH; break;
				case EM_TOKEN_TYPE_BITWISE_RIGHT_SHIFT: op = EM_CODE_OP_BBRSH; break;
				case EM_TOKEN_TYPE_DOUBLE_EQUALS: op = EM_CODE_OP_BEQ; break;
				case EM_TOKEN_TYPE_NOT_EQUALS: op = EM_CODE_OP_BNEQ; break;
				case EM_TOKEN_TYPE_LESS_THAN: op = EM_CODE_OP_BLT; break;
				case EM_TOKEN_TYPE_GREATER_THAN: op = EM_CODE_OP_BGT; break;
			}
			em_code_write_uint8(slice, (uint8_t)op);
			break;

		/* member access */
		case EM_NODE_TYPE_ACCESS:
			if (node->first->next) { /* indexed */

				em_code_write(compiler, node->first);
				em_code_write(compiler, node->first->next);
				set_position(compiler, node);
				em_code_write_uint8(slice, EM_CODE_OP_LDIDX);
			}
			else { /* named */
				em_code_write(compiler, node->first);
				set_position(compiler, node);
				em_code_write_uint8(slice, EM_CODE_OP_LDNM);

				token = em_node_get_token(node, 0);
				hash = em_node_get_value(node, 0).v.te_hash;

				em_code_write_hashed_string(
						slice, token->value,
						token->length, hash
				);
			}
			break;

		/* call */
		case EM_NODE_TYPE_CALL:
			for (node = node->first; node; node = node->next) {

				em_code_write(compiler, node);
				count++;
			}
			node = orig;
			set_position(compiler, node);
			em_code_write_uint8(slice, EM_CODE_OP_CALL);
			em_code_write_uint16(slice, (uint16_t)count);
			break;

		/* continue */
		case EM_NODE_TYPE_CONTINUE:
			set_position(compiler, node);
			em_code_write_uint8(slice, EM_CODE_OP_PTRUE);
			em_code_write_uint8(slice, EM_CODE_OP_RSTR3);
			break;

		/* break */
		case EM_NODE_TYPE_BREAK:
			set_position(compiler, node);
			em_code_write_uint8(slice, EM_CODE_OP_PFLSE);
			em_code_write_uint8(slice, EM_CODE_OP_RSTR3);
			break;

		/* return */
		case EM_NODE_TYPE_RETURN:
			em_code_write(compiler, node->first);
			set_position(compiler, node);
			em_code_write_uint8(slice, EM_CODE_OP_RSTR2);
			break;

		/* raise */
		case EM_NODE_TYPE_RAISE:
			em_code_write(compiler, node->first);
			set_position(compiler, node);
			em_code_write_uint8(slice, EM_CODE_OP_RSTR1);
			break;

		/* include */
		case EM_NODE_TYPE_INCLUDE:
			em_code_write(compiler, node->first);
			set_position(compiler, node);
			em_code_write_uint8(slice, EM_CODE_OP_INCLUDE);
			break;

		/* let statement */
		case EM_NODE_TYPE_LET:
			if (node->first->next) { /* indexed */

				set_position(compiler, node);
				for (size_t i = 0; i < node->tokens.nitems; i++) {

					token = em_node_get_token(node, i);
					hash = em_node_get_value(node, i).v.te_hash;

					op = EM_CODE_OP_LOAD;
					if (i) op = EM_CODE_OP_LDNM;

					em_code_write_uint8(slice, op);
					em_code_write_hashed_string(
							slice, token->value,
							token->length, hash
					);
				}
				em_code_write(compiler, node->first);
				em_code_write(compiler, node->first->next);
				set_position(compiler, node);
				em_code_write_uint8(slice, EM_CODE_OP_STIDX);
			}
			else { /* named */
				set_position(compiler, node);
				for (size_t i = 0; i < node->tokens.nitems-1; i++) {

					token = em_node_get_token(node, i);
					hash = em_node_get_value(node, i).v.te_hash;

					op = EM_CODE_OP_LOAD;
					if (i) op = EM_CODE_OP_LDNM;

					em_code_write_uint8(slice, op);
					em_code_write_hashed_string(
							slice, token->value,
							token->length, hash
					);
				}
				em_code_write(compiler, node->first);
				set_position(compiler, node);

				token = em_node_get_token(node, node->tokens.nitems-1);
				hash = em_node_get_value(node, node->tokens.nitems-1).v.te_hash;

				if (node->tokens.nitems > 1)
					em_code_write_uint8(slice, EM_CODE_OP_STNM);
				else
					em_code_write_uint8(slice, EM_CODE_OP_STOR);
				em_code_write_hashed_string(
						slice, token->value,
						token->length, hash
				);
			}
			break;

		/* if statement */
		case EM_NODE_TYPE_IF:
			count = slice->position;
			node = node->first;
			while (node) {

				em_node_t *condition_node = node;
				em_node_t *body_node = condition_node->next;

				if (!body_node) {

					body_node = condition_node;
					condition_node = NULL;
				}
				if (condition_node) {

					em_code_write(compiler, condition_node);

					int32_t jump = (int32_t)body_node->code_size;
					if (body_node->next) jump += 5;

					em_code_write_uint8(slice, EM_CODE_OP_JNTR);
					em_code_write_int32(slice, jump);
				}
				em_code_write(compiler, body_node);
				if (body_node->next) {

					em_code_write_uint8(slice, EM_CODE_OP_JMP);

					int32_t jump = PC_REL(slice, count + orig->code_size);
					em_code_write_int32(slice, jump);
				}

				node = body_node->next;
			}
			break;

		/* for statement */
		case EM_NODE_TYPE_FOR:
			token = em_node_get_token(node, 0);
			hash = em_utf8_strhash(token->value);
			count = HASH_STR_SIZE(token->length);

			pos_a = slice->position + node->first->code_size + 12 + count; /* @break */
			pos_b = pos_a + 11; /* @start */
			pos_c = pos_b + 3 + count * 2; /* @cond */
			pos_d = pos_c + node->first->next->code_size + 7; /* @body */
			pos_e = pos_d + node->first->next->next->code_size + 5; /* @end */

			/* @init */
			em_code_write_uint8(slice, EM_CODE_OP_SAVE3);
			em_code_write_int32(slice, PC_REL(slice, pos_a)); /* SAVE3 @break */
			em_code_write_uint8(slice, EM_CODE_OP_PNONE);
			em_code_write(compiler, node->first);
			em_code_write_uint8(slice, EM_CODE_OP_STOR);
			em_code_write_hashed_string(
					slice, token->value,
					token->length, hash
			);
			em_code_write_uint8(slice, EM_CODE_OP_JMP);
			em_code_write_int32(slice, PC_REL(slice, pos_c)); /* JMP @cond */
			/* @break */
			em_code_write_uint8(slice, EM_CODE_OP_JPNTR);
			em_code_write_int32(slice, PC_REL(slice, pos_e+1)); /* JNTR @end+1 */
			em_code_write_uint8(slice, EM_CODE_OP_SAVE3);
			em_code_write_int32(slice, PC_REL(slice, pos_a)); /* SAVE3 @break */
			em_code_write_uint8(slice, EM_CODE_OP_PNONE);
			/* @start */
			em_code_write_uint8(slice, EM_CODE_OP_LOAD);
			em_code_write_hashed_string(
					slice, token->value,
					token->length, hash
			);
			em_code_write_uint8(slice, EM_CODE_OP_UINC);
			em_code_write_uint8(slice, EM_CODE_OP_STOR);
			em_code_write_hashed_string(
					slice, token->value,
					token->length, hash
			);
			/* @cond */
			em_code_write(compiler, node->first->next);
			em_code_write_uint8(slice, EM_CODE_OP_BLT);
			em_code_write_uint8(slice, EM_CODE_OP_JNTR);
			em_code_write_int32(slice, PC_REL(slice, pos_e)); /* JNTR @end */
			em_code_write_uint8(slice, EM_CODE_OP_POP);
			/* @body */
			em_code_write(compiler, node->first->next->next);
			em_code_write_uint8(slice, EM_CODE_OP_JMP);
			em_code_write_int32(slice, PC_REL(slice, pos_b)); /* JMP @start */
			/* @end */
			em_code_write_uint8(slice, EM_CODE_OP_DSCD3);
			break;

		/* foreach statement */
		case EM_NODE_TYPE_FOREACH:
			token = em_node_get_token(node, 0);
			hash = em_utf8_strhash(token->value);
			count = HASH_STR_SIZE(token->length);

			pos_a = slice->position + 13 + node->first->code_size; /* @break */
			pos_b = pos_a + 11; /* @start */
			pos_c = pos_b + 7 + count; /* @body */
			pos_d = pos_c + 5 + node->first->next->code_size; /* @breakend */
			pos_e = pos_d + 9; /* @end */

			/* @init */
			em_code_write(compiler, node->first);
			em_code_write_uint8(slice, EM_CODE_OP_LEN);
			em_code_write_uint8(slice, EM_CODE_OP_PFLSE);
			em_code_write_uint8(slice, EM_CODE_OP_PNONE);
			em_code_write_uint8(slice, EM_CODE_OP_SAVE3);
			em_code_write_int32(slice, PC_REL(slice, pos_a)); /* SAVE3 @break */
			em_code_write_uint8(slice, EM_CODE_OP_JMP);
			em_code_write_int32(slice, PC_REL(slice, pos_b)); /* JMP @start */
			/* @break */
			em_code_write_uint8(slice, EM_CODE_OP_JNTR);
			em_code_write_int32(slice, PC_REL(slice, pos_d)); /* JNTR @breakend */
			em_code_write_uint8(slice, EM_CODE_OP_SAVE3);
			em_code_write_int32(slice, PC_REL(slice, pos_a)); /* SAVE3 @break */
			em_code_write_uint8(slice, EM_CODE_OP_PNONE);
			/* @start */
			em_code_write_uint8(slice, EM_CODE_OP_BLTJXPIPI);
			em_code_write_int32(slice, PC_REL(slice, pos_e)); /* BLTJXPIPI @end */
			em_code_write_uint8(slice, EM_CODE_OP_STOR);
			em_code_write_hashed_string(
					slice, token->value,
					token->length, hash
			);
			em_code_write_uint8(slice, EM_CODE_OP_POP);
			/* @body */
			em_code_write(compiler, node->first->next);
			em_code_write_uint8(slice, EM_CODE_OP_JMP);
			em_code_write_int32(slice, PC_REL(slice, pos_b)); /* JMP @start */
			/* @breakend */
			em_code_write_uint8(slice, EM_CODE_OP_POP);
			em_code_write_uint8(slice, EM_CODE_OP_POP);
			em_code_write_uint8(slice, EM_CODE_OP_POP);
			em_code_write_uint8(slice, EM_CODE_OP_PNONE);
			em_code_write_uint8(slice, EM_CODE_OP_JMP);
			em_code_write_int32(slice, 1);
			/* @end */
			em_code_write_uint8(slice, EM_CODE_OP_DSCD3);
			break;

		/* while statement */
		case EM_NODE_TYPE_WHILE:
			pos_a = slice->position + 11; /* @break */
			pos_b = pos_a + 11; /* @start */
			pos_c = pos_b + 6 + node->first->code_size; /* @body */
			pos_d = pos_c + 5 + node->first->next->code_size; /* @end */

			/* @init */
			em_code_write_uint8(slice, EM_CODE_OP_PNONE);
			em_code_write_uint8(slice, EM_CODE_OP_SAVE3);
			em_code_write_int32(slice, PC_REL(slice, pos_a)); /* SAVE3 @break */
			em_code_write_uint8(slice, EM_CODE_OP_JMP);
			em_code_write_int32(slice, PC_REL(slice, pos_b)); /* JMP @start */
			/* @break */
			em_code_write_uint8(slice, EM_CODE_OP_JPNTR);
			em_code_write_int32(slice, PC_REL(slice, pos_d+1)); /* JNTR @end+1 */
			em_code_write_uint8(slice, EM_CODE_OP_SAVE3);
			em_code_write_int32(slice, PC_REL(slice, pos_a)); /* SAVE3 @break */
			em_code_write_uint8(slice, EM_CODE_OP_PNONE);
			/* @start */
			em_code_write(compiler, node->first);
			em_code_write_uint8(slice, EM_CODE_OP_JNTR);
			em_code_write_int32(slice, PC_REL(slice, pos_d)); /* JNTR @end */
			em_code_write_uint8(slice, EM_CODE_OP_POP);
			/* @body */
			em_code_write(compiler, node->first->next);
			em_code_write_uint8(slice, EM_CODE_OP_JMP);
			em_code_write_int32(slice, PC_REL(slice, pos_b)); /* JMP @start */
			/* @end */
			em_code_write_uint8(slice, EM_CODE_OP_DSCD3);
			break;

		/* func statement */
		case EM_NODE_TYPE_FUNC:
			em_code_write_uint8(slice, EM_CODE_OP_DFUNC);
			em_code_write_uint8(slice, node->tokens.nitems - (node->flags? 1: 0));

			if (node->flags) {

				token = em_node_get_token(node, 0);
				hash = em_utf8_strhash(token->value);

				em_code_write_hashed_string(
						slice, token->value,
						token->length, hash
				);
			}
			else em_code_write_hashed_string(
					slice, "<anonymous>",
					11, 0xc513fead
			);
			for (size_t i = 0; i < node->tokens.nitems; i++) {

				if (!i && node->flags)
					continue;
				token = em_node_get_token(node, i);
				hash = em_utf8_strhash(token->value);

				em_code_write_hashed_string(
						slice, token->value,
						token->length, hash
				);
			}
			em_code_write_uint32(slice, (uint32_t)node->first->code_size);
			em_code_write(compiler, node->first);

			if (node->flags) {

				token = em_node_get_token(node, 0);
				hash = em_utf8_strhash(token->value);

				em_code_write_uint8(slice, EM_CODE_OP_STOR);
				em_code_write_hashed_string(
						slice, token->value,
						token->length, hash
				);
			}
			break;

		/* class statement */
		case EM_NODE_TYPE_CLASS:
			token = em_node_get_token(node, 0);
			hash = em_utf8_strhash(token->value);

			if (node->first->next) { /* with base class */

				em_code_write(compiler, node->first);
				em_code_write_uint8(slice, EM_CODE_OP_DBGN);
				em_code_write(compiler, node->first->next);
			}
			else { /* without base class */
				em_code_write_uint8(slice, EM_CODE_OP_PNONE);
				em_code_write_uint8(slice, EM_CODE_OP_DBGN);
				em_code_write(compiler, node->first);
			}
			em_code_write_uint8(slice, EM_CODE_OP_DCLS);
			em_code_write_hashed_string(
					slice, token->value,
					token->length, hash
			);
			break;

		/* try statement */
		case EM_NODE_TYPE_TRY:
			token = em_node_get_token(node, 0);
			hash = em_utf8_strhash(token->value);
			count = HASH_STR_SIZE(token->length);

			pos_a = slice->position + 5; /* @try */
			pos_b = pos_a + 5 + node->first->code_size; /* @catch */
			pos_c = pos_b + 8 + node->first->next->code_size;
			pos_c += count + node->first->next->next->code_size; /* @end */

			/* @init */
			em_code_write_uint8(slice, EM_CODE_OP_SAVE1);
			em_code_write_int32(slice, PC_REL(slice, pos_b)); /* SAVE1 @catch */
			/* @try */
			em_code_write(compiler, node->first);
			em_code_write_uint8(slice, EM_CODE_OP_JMP);
			em_code_write_int32(slice, PC_REL(slice, pos_c)); /* JMP @end */
			/* @catch */
			em_code_write(compiler, node->first->next);
			em_code_write_uint8(slice, EM_CODE_OP_R1EISNTP);
			em_code_write_uint8(slice, EM_CODE_OP_STOR);
			em_code_write_hashed_string(
					slice, token->value,
					token->length, hash
			);
			em_code_write_uint8(slice, EM_CODE_OP_POP);
			em_code_write(compiler, node->first->next->next);
			em_code_write_uint8(slice, EM_CODE_OP_JMP);
			em_code_write_int32(slice, PC_REL(slice, pos_c+1)); /* JMP @end+1 */
			/* @end */
			em_code_write_uint8(slice, EM_CODE_OP_DSCD1);
			break;
	}
}

/* disassemble generated code */
EM_API void em_code_disassemble(em_code_slice_t *slice, FILE *fp) {

	slice->position = 0;
	em_hash_t hash;
	uint8_t count;

	while (slice->position < slice->length) {

		fprintf(fp, "%08x  ", slice->position);
		em_code_op_t op = (em_code_op_t)em_code_read_uint8(slice);

		switch (op) {

			/* push int constant */
			case EM_CODE_OP_PCINT:
				fprintf(fp,
					"PCINT " EM_INTTYPE_FORMAT "\n",
					em_code_read_inttype(slice));
				break;

			/* push float constant */
			case EM_CODE_OP_PCFLT:
				fprintf(fp,
					"PCFLT " EM_FLOATTYPE_FORMAT "\n",
					em_code_read_floattype(slice));
				break;

			/* push string constant */
			case EM_CODE_OP_PCSTR:
				fprintf(fp,
					"PCSTR \"%s\"\n",
					em_code_read_string(slice));
				break;

			/* set line */
			case EM_CODE_OP_ESETL:
				fprintf(fp,
					"ESETL %hu\n",
					em_code_read_uint16(slice));
				break;

			/* set column */
			case EM_CODE_OP_ESETC:
				fprintf(fp,
					"ESETC %hhu\n",
					em_code_read_uint8(slice));
				break;

			/* single word instructions */
			case EM_CODE_OP_PTRUE:
			case EM_CODE_OP_PFLSE:
			case EM_CODE_OP_PNONE:
			case EM_CODE_OP_POP:
			case EM_CODE_OP_UNEG:
			case EM_CODE_OP_UNOT:
			case EM_CODE_OP_UBNOT:
			case EM_CODE_OP_UINC:
			case EM_CODE_OP_UDEC:
			case EM_CODE_OP_BADD:
			case EM_CODE_OP_BSUB:
			case EM_CODE_OP_BMUL:
			case EM_CODE_OP_BDIV:
			case EM_CODE_OP_BMOD:
			case EM_CODE_OP_BBOR:
			case EM_CODE_OP_BBXOR:
			case EM_CODE_OP_BBAND:
			case EM_CODE_OP_BBLSH:
			case EM_CODE_OP_BBRSH:
			case EM_CODE_OP_BEQ:
			case EM_CODE_OP_BNEQ:
			case EM_CODE_OP_BLT:
			case EM_CODE_OP_BGT:
			case EM_CODE_OP_LDIDX:
			case EM_CODE_OP_STIDX:
			case EM_CODE_OP_RSTR1:
			case EM_CODE_OP_RSTR2:
			case EM_CODE_OP_RSTR3:
			case EM_CODE_OP_DSCD1:
			case EM_CODE_OP_DSCD3:
			case EM_CODE_OP_DBGN:
			case EM_CODE_OP_INCLUDE:
			case EM_CODE_OP_LEN:
			case EM_CODE_OP_R1EISNTP:
				fprintf(fp, "%s\n", op_names[op]);
				break;

			/* jumps */
			case EM_CODE_OP_JMP:
			case EM_CODE_OP_JTR:
			case EM_CODE_OP_JNTR:
			case EM_CODE_OP_JPNTR:
			case EM_CODE_OP_SAVE1:
			case EM_CODE_OP_SAVE3:
			case EM_CODE_OP_BLTJXPIPI:
				fprintf(fp, "%s %+d\n", op_names[op],
					em_code_read_int32(slice));
				break;

			/* constructors and calls */
			case EM_CODE_OP_CLIST:
			case EM_CODE_OP_CMAP:
			case EM_CODE_OP_CALL:
			case EM_CODE_OP_PUTS:
				fprintf(fp, "%s %hu\n", op_names[op],
					em_code_read_uint16(slice));
				break;

			/* loads and stores */
			case EM_CODE_OP_LOAD:
			case EM_CODE_OP_LDNM:
			case EM_CODE_OP_STOR:
			case EM_CODE_OP_STNM:
				fprintf(fp, "%s \"%s\"\n", op_names[op],
					em_code_read_hashed_string(slice, &hash));
				break;

			/* define function */
			case EM_CODE_OP_DFUNC:
				count = em_code_read_uint8(slice);
				fprintf(fp, "DFUNC \"%s\" (",
					em_code_read_hashed_string(slice, &hash));
				for (uint8_t i = 0; i < count; i++) {

					if (i) fputs(", ", fp);
					fprintf(fp, "\"%s\"",
						em_code_read_hashed_string(slice, &hash));
				}
				fprintf(fp, ") +%u\n",
					em_code_read_uint32(slice));
				break;

			/* define class */
			case EM_CODE_OP_DCLS:
				fprintf(fp, "DCLS \"%s\"\n",
					em_code_read_hashed_string(slice, &hash));
				break;

			/* otherwise */
			default:
				fprintf(fp, "Unknown (0x%x)\n", op);
				slice->position = 0;
				return;
		}
	}
	slice->position = 0;
}

/* recover context for level, within slice */
static em_bool_t recover_context(
		em_context_t *context,
		em_code_slice_t *slice,
		em_code_op_t op_level
) {
	uint32_t level = 0;
	switch (op_level) {
		case EM_CODE_OP_RSTR1: level = 1; break;
		case EM_CODE_OP_RSTR2: level = 2; break;
		case EM_CODE_OP_RSTR3: level = 3; break;
	}

	for (size_t i = context->csp; i; i--) {

		if (context->cstack[i-1].level == level) {

			slice->position = context->cstack[i-1].pos;

			size_t old_sp = context->sp;
			context->sp = context->cstack[i-1].sp;

			for (size_t i = context->sp; i < old_sp; i++)
				em_value_delete(context->stack[i]);

			context->csp = i-1;
			return EM_TRUE;
		}
		else if (context->cstack[i-1].level == 2)
			break;
	}
	return EM_FALSE;
}

/* run code slice */
EM_API em_value_t em_code_run_slice(em_context_t *context, em_code_slice_t *slice) {

	slice->position = 0;
	slice->mode = EM_CODE_OP_CALL;

	while (slice->position < slice->length) {

		em_code_run_inst(context, slice);

		/* restore context */
		size_t old_sp = context->sp;
		em_value_t value = em_context_pop_value(context);

		if (slice->mode != EM_CODE_OP_CALL &&
		    !recover_context(context, slice, slice->mode)) {

			context->pass = value;
			context->mode = slice->mode;
			return EM_VALUE_FAIL;
		}
		else if (old_sp) em_context_push_value(context, value);
		slice->mode = EM_CODE_OP_CALL;
	}
	return em_context_pop_value(context);
}

/* run single instruction */
#define FAIL ({\
		context->pass = EM_VALUE_FAIL;\
		slice->mode = EM_CODE_OP_RSTR1;\
		return;\
	})

#define RUNTIME_ERROR(...) ({\
	if (!em_log_catch(NULL))\
		em_log_runtime_error(&context->op_pos, __VA_ARGS__);\
	context->pass = EM_VALUE_FAIL;\
	slice->mode = EM_CODE_OP_RSTR1;\
	return;\
})

#define UNARY_OPERATION(p_op, ...) ({\
	a = em_context_pop_value(context);\
	b = em_value_##p_op;\
	em_value_delete(a);\
	if (!EM_VALUE_OK(b)) FAIL;\
	__VA_ARGS__;\
	em_context_push_value(context, b);\
})

#define BINARY_OPERATION(p_name, ...) ({\
	b = em_context_pop_value(context);\
	a = em_context_pop_value(context);\
	c = em_value_##p_name(a, b, &context->op_pos);\
	em_value_delete(a);\
	em_value_delete(b);\
	if (!EM_VALUE_OK(c)) FAIL;\
	__VA_ARGS__;\
	em_context_push_value(context, c);\
})

EM_API void em_code_run_inst(em_context_t *context, em_code_slice_t *slice) {

	em_code_op_t op = (em_code_op_t)em_code_read_uint8(slice);
	em_value_t a, b, c;
	size_t count;
	em_hash_t hash;
	const char *string;
	em_result_t result;

	switch (op) {

		/* push constants */
		case EM_CODE_OP_PCINT:
			em_context_push_value(
				context,
				EM_VALUE_INT(em_code_read_inttype(slice))
			);
			break;
		case EM_CODE_OP_PCFLT:
			em_context_push_value(
				context,
				EM_VALUE_INT(em_code_read_floattype(slice))
			);
			break;
		case EM_CODE_OP_PCSTR:
			string = em_code_read_string(slice);
			em_context_push_value(
				context,
				em_string_new_from_utf8(string, em_utf8_strlen(string))
			);
			break;
		case EM_CODE_OP_PTRUE:
			em_context_push_value(context, EM_VALUE_TRUE);
			break;
		case EM_CODE_OP_PFLSE:
			em_context_push_value(context, EM_VALUE_FALSE);
			break;
		case EM_CODE_OP_PNONE:
			em_context_push_value(context, em_none);
			break;

		/* remove value */
		case EM_CODE_OP_POP:
			em_value_delete(em_context_pop_value(context));
			break;

		/* construct list */
		case EM_CODE_OP_CLIST:
			count = (size_t)em_code_read_uint16(slice);
			a = em_list_new(count);

			for (size_t i = 0; i < count; i++) {

				b = context->stack[context->sp-count+i];
				em_list_append(a, b);
			}
			for (size_t i = 0; i < count; i++)
				em_value_delete(em_context_pop_value(context));
			em_context_push_value(context, a);
			break;

		/* unary operations */
		case EM_CODE_OP_UNEG:
			UNARY_OPERATION(multiply(a, EM_VALUE_INT(-1), &context->op_pos));
			break;
		case EM_CODE_OP_UNOT:
			UNARY_OPERATION(is_true(a, &context->op_pos), b = EM_VALUE_INT_INV(b));
			break;
		case EM_CODE_OP_UBNOT:
			UNARY_OPERATION(not(a, &context->op_pos));
			break;
		case EM_CODE_OP_UINC:
			UNARY_OPERATION(add(a, EM_VALUE_INT(1), &context->op_pos));
			break;
		case EM_CODE_OP_UDEC:
			UNARY_OPERATION(subtract(a, EM_VALUE_INT(-1), &context->op_pos));
			break;

		/* binary operations */
		case EM_CODE_OP_BADD:
			BINARY_OPERATION(add);
			break;
		case EM_CODE_OP_BSUB:
			BINARY_OPERATION(subtract);
			break;
		case EM_CODE_OP_BMUL:
			BINARY_OPERATION(multiply);
			break;
		case EM_CODE_OP_BDIV:
			BINARY_OPERATION(divide);
			break;
		case EM_CODE_OP_BMOD:
			BINARY_OPERATION(modulo);
			break;
		case EM_CODE_OP_BBOR:
			BINARY_OPERATION(or);
			break;
		case EM_CODE_OP_BBXOR:
			BINARY_OPERATION(xor);
			break;
		case EM_CODE_OP_BBAND:
			BINARY_OPERATION(and);
			break;
		case EM_CODE_OP_BBLSH:
			BINARY_OPERATION(shift_left);
			break;
		case EM_CODE_OP_BBRSH:
			BINARY_OPERATION(shift_right);
			break;
		case EM_CODE_OP_BEQ:
			BINARY_OPERATION(compare_equal);
			break;
		case EM_CODE_OP_BNEQ:
			BINARY_OPERATION(compare_equal, c = EM_VALUE_INT_INV(c));
			break;
		case EM_CODE_OP_BLT:
			BINARY_OPERATION(compare_less_than);
			break;
		case EM_CODE_OP_BGT:
			BINARY_OPERATION(compare_greater_than);
			break;

		/* load value */
		case EM_CODE_OP_LOAD:
			string = em_code_read_hashed_string(slice, &hash);

			a = em_context_get_value(context, hash);
			if (!EM_VALUE_OK(a))
				RUNTIME_ERROR("Variable '%s' not defined", string);
			em_context_push_value(context, a);
			break;

		/* load value at index */
		case EM_CODE_OP_LDIDX:
			b = em_context_pop_value(context);
			a = em_context_pop_value(context);

			c = em_value_get_by_index(a, b, &context->op_pos);
			em_value_delete(a);
			em_value_delete(b);

			if (!EM_VALUE_OK(c))
				RUNTIME_ERROR("Invalid index");
			em_context_push_value(context, c);
			break;

		/* store value */
		case EM_CODE_OP_STOR:
			string = em_code_read_hashed_string(slice, &hash);
			a = em_context_pop_value(context);

			em_context_set_value(context, hash, a);
			em_context_push_value(context, a);
			break;

		/* store value at index */
		case EM_CODE_OP_STIDX:
			c = em_context_pop_value(context);
			b = em_context_pop_value(context);
			a = em_context_pop_value(context);

			result = em_value_set_by_index(a, b, c, &context->op_pos);
			em_value_delete(a);
			em_value_delete(b);

			if (result != EM_RESULT_SUCCESS)
				RUNTIME_ERROR("Invalid index");
			em_context_push_value(context, c);
			break;

		/* jump to position */
		case EM_CODE_OP_JMP:
			count = (size_t)em_code_read_int32(slice);
			slice->position += count;
			break;

		/* jump to position if true */
		case EM_CODE_OP_JTR:
			count = (size_t)em_code_read_int32(slice);
			a = em_context_pop_value(context);
			b = em_value_is_true(a, &context->op_pos);

			if (b.value.te_inttype)
				slice->position += count;
			em_value_delete(a);
			break;

		/* jump to position if not true */
		case EM_CODE_OP_JNTR:
			count = (size_t)em_code_read_int32(slice);
			a = em_context_pop_value(context);
			b = em_value_is_true(a, &context->op_pos);

			if (!b.value.te_inttype)
				slice->position += count;
			em_value_delete(a);
			break;

		/* jump to position and push none if not true */
		case EM_CODE_OP_JPNTR:
			count = (size_t)em_code_read_int32(slice);
			a = em_context_pop_value(context);
			b = em_value_is_true(a, &context->op_pos);
			em_value_delete(a);

			if (!b.value.te_inttype) {

				slice->position += count;
				em_context_push_value(context, em_none);
			}
			break;

		/* save context */
		case EM_CODE_OP_SAVE3:
			count = (size_t)em_code_read_int32(slice);
			em_context_push_context(
					context,
					1,
					slice->position + count,
					context->sp
			);
			break;

		/* discard context */
		case EM_CODE_OP_DSCD1:
		case EM_CODE_OP_DSCD3:
			if (context->csp) context->csp--;
			break;

		/* set error position */
		case EM_CODE_OP_ESETL:
			context->op_pos.line = (em_ssize_t)
				em_code_read_uint16(slice);
			break;
		case EM_CODE_OP_ESETC:
			context->op_pos.column = (em_ssize_t)
				em_code_read_uint8(slice);
			break;

		/* print values */
		case EM_CODE_OP_PUTS:
			count = (size_t)em_code_read_uint16(slice);
			for (size_t i = 0; i < count; i++) {

				if (i) fputc(' ', stdout);

				em_value_t value = context->stack[context->sp-count+i];
				em_value_t string = em_value_to_string(value, &context->op_pos);

				if (!EM_VALUE_OK(string))
					FAIL;
				em_string_t *strobject =
					EM_STRING(EM_OBJECT_FROM_VALUE(string));
				em_wchar_write(stdout, strobject->data, strobject->length);

				if (!em_value_is(value, string))
					em_value_delete(string);
			}
			a = em_context_pop_value(context);
			for (size_t i = 0; i < count-1; i++)
				em_value_delete(em_context_pop_value(context));
			em_context_push_value(context, a);
			fputc('\n', stdout);
			break;

		/* unknown operation */
		default:
			RUNTIME_ERROR("Unknown / unimplemented operation (0x%x)", op);
			break;
	}
}
