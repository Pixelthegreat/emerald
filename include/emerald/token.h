/*
 * Copyright 2025, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef EMERALD_TOKEN_H
#define EMERALD_TOKEN_H

#include <emerald/core.h>
#include <emerald/log.h>
#include <emerald/main.h>
#include <emerald/refobj.h>

/* token types */
typedef enum em_token_type {
	EM_TOKEN_TYPE_NONE = 0,

	/* compound tokens */
	EM_TOKEN_TYPE_INT,
	EM_TOKEN_TYPE_FLOAT,
	EM_TOKEN_TYPE_IDENTIFIER,
	EM_TOKEN_TYPE_KEYWORD,
	EM_TOKEN_TYPE_STRING,

	/* single character tokens */
	EM_TOKEN_TYPE_PLUS,
	EM_TOKEN_TYPE_MINUS,
	EM_TOKEN_TYPE_ASTERISK,
	EM_TOKEN_TYPE_SLASH,
	EM_TOKEN_TYPE_OPEN_PAREN,
	EM_TOKEN_TYPE_CLOSE_PAREN,
	EM_TOKEN_TYPE_OPEN_SQUARE_BRACKET,
	EM_TOKEN_TYPE_CLOSE_SQUARE_BRACKET,
	EM_TOKEN_TYPE_COMMA,
	EM_TOKEN_TYPE_DOT,
	EM_TOKEN_TYPE_EQUALS,
	EM_TOKEN_TYPE_LESS_THAN,
	EM_TOKEN_TYPE_GREATER_THAN,
	
	/* multiple character tokens */
	EM_TOKEN_TYPE_DOUBLE_EQUALS,
	EM_TOKEN_TYPE_NOT_EQUALS,
	EM_TOKEN_TYPE_LESS_THAN_EQUALS,
	EM_TOKEN_TYPE_GREATER_THAN_EQUALS,
	EM_TOKEN_TYPE_BITWISE_LEFT_SHIFT,
	EM_TOKEN_TYPE_BITWISE_RIGHT_SHIFT,
	EM_TOKEN_TYPE_BITWISE_AND,
	EM_TOKEN_TYPE_BITWISE_OR,
	EM_TOKEN_TYPE_BITWISE_NOT,

	EM_TOKEN_TYPE_EOF,
	EM_TOKEN_TYPE_COUNT,
} em_token_type_t;

/* token */
typedef struct em_token {
	em_refobj_t base;
	struct em_token *next; /* next token */
	em_token_type_t type; /* token type */
	em_pos_t pos; /* token position */
	char value[]; /* token value */
} em_token_t;

#define EM_TOKEN(p) ((em_token_t *)(p))

EM_API em_reflist_t em_reflist_token;

/* functions */
EM_API const char *em_get_token_type_name(em_token_type_t type); /* get name of token type */

EM_API em_token_t *em_token_new(em_token_type_t type, em_pos_t *pos, const char *value, size_t len); /* create token */
EM_API void em_token_print(em_token_t *token); /* print token list (debug) */

#endif /* EMERALD_TOKEN_H */
