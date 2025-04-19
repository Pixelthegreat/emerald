/*
 * Copyright 2025, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <emerald/core.h>
#include <emerald/utf8.h>
#include <emerald/log.h>
#include <emerald/lexer.h>

/* predictable ctype alternatives */
#define ISDIGIT(c) (((c) >= '0') && ((c) <= '9'))
#define ISIDENT(c) ((((c) >= 'a') && ((c) <= 'z')) || (((c) >= 'A') && (c <= 'Z')) || ((c) == '_'))
#define ISIDENT_A(c) (ISIDENT(c) || ISDIGIT(c))
#define ISDELIM(c) (((c) == '\'') || ((c) == '"'))

/* keywords list */
static const char *keywords[] = {
	"if", "elif", "else",
	"and", "or", "not",
	"for", "foreach", "while", "to", "in",
	"let", "include", "puts", "gets", "then", "end", "return", "try", "catch", "raise",
	"func", "class",
};
#define N_KEYWORDS (sizeof(keywords) / sizeof(const char *))

/* identify an escape character */
static int getescchar(int c) {

	switch (c) {
		case 'n': return '\n';
		case 'r': return '\r';
		case 't': return '\t';
		case 'e': return '\x1b';
		default: return c;
	}
}

/* initialize lexer */
EM_API em_result_t em_lexer_init(em_lexer_t *lexer) {

	if (!lexer) return EM_RESULT_FAILURE;
	else if (lexer->init) {

		em_log_fatal("Lexer already initialized");
		return EM_RESULT_FAILURE;
	}

	/* initialize */
	lexer->pos = EM_POS_INIT;
	lexer->first = NULL;
	lexer->last = NULL;

	lexer->init = EM_TRUE;
	return EM_RESULT_SUCCESS;
}

/* reset lexer */
EM_API void em_lexer_reset(em_lexer_t *lexer, const char *path, const char *text, em_ssize_t len) {

	if (!lexer || !lexer->init) return;

	em_token_t *cur = lexer->first;
	while (cur) {

		em_token_t *next = cur->next;

		cur->next = NULL;
		em_refobj_decref(EM_REFOBJ(cur));

		cur = next;
	}

	lexer->first = NULL;
	lexer->last = NULL;

	lexer->pos = EM_POS_INIT;
	lexer->pos.path = path;
	lexer->pos.text = text;
	lexer->pos.len = len;
	em_pos_advance(&lexer->pos);
}

/* add token with length specified */
EM_API em_token_t *em_lexer_add_token_full(em_lexer_t *lexer, em_token_type_t type, em_pos_t *pos, const char *value, size_t len) {

	em_token_t *token = em_token_new(type, pos, value, len);
	if (!token) return NULL;

	if (!lexer->first) lexer->first = token;
	if (lexer->last) lexer->last->next = token;
	lexer->last = token;

	return token;
}

/* add token */
EM_API em_token_t *em_lexer_add_token(em_lexer_t *lexer, em_token_type_t type, em_pos_t *pos, const char *value) {

	return em_lexer_add_token_full(lexer, type, pos, value, strlen(value));
}

/* generate tokens from input text */
EM_API em_result_t em_lexer_make_tokens(em_lexer_t *lexer) {

	while (lexer->pos.cc) {

		/* whitespace */
		if (strchr(" \t\n", lexer->pos.cc))
			em_pos_advance(&lexer->pos);

		/* comment */
		else if (lexer->pos.cc == '#') {

			while (lexer->pos.cc && lexer->pos.cc != '\n')
				em_pos_advance(&lexer->pos);
		}

		/* make a number */
		else if (ISDIGIT(lexer->pos.cc)) {

			if (em_lexer_make_number(lexer) != EM_RESULT_SUCCESS)
				return EM_RESULT_FAILURE;
		}

		/* make an identifier */
		else if (ISIDENT(lexer->pos.cc)) {

			if (em_lexer_make_identifier(lexer) != EM_RESULT_SUCCESS)
				return EM_RESULT_FAILURE;
		}

		/* make a string */
		else if (ISDELIM(lexer->pos.cc)) {

			if (em_lexer_make_string(lexer) != EM_RESULT_SUCCESS)
				return EM_RESULT_FAILURE;
		}

		/* plus */
		else if (lexer->pos.cc == '+') {

			if (!em_lexer_add_token(lexer, EM_TOKEN_TYPE_PLUS, &lexer->pos, "+"))
				return EM_RESULT_FAILURE;
			em_pos_advance(&lexer->pos);
		}

		/* minus */
		else if (lexer->pos.cc == '-') {

			if (!em_lexer_add_token(lexer, EM_TOKEN_TYPE_MINUS, &lexer->pos, "-"))
				return EM_RESULT_FAILURE;
			em_pos_advance(&lexer->pos);
		}

		/* asterisk */
		else if (lexer->pos.cc == '*') {

			if (!em_lexer_add_token(lexer, EM_TOKEN_TYPE_ASTERISK, &lexer->pos, "*"))
				return EM_RESULT_FAILURE;
			em_pos_advance(&lexer->pos);
		}

		/* slash */
		else if (lexer->pos.cc == '/') {

			if (!em_lexer_add_token(lexer, EM_TOKEN_TYPE_SLASH, &lexer->pos, "/"))
				return EM_RESULT_FAILURE;
			em_pos_advance(&lexer->pos);
		}

		/* open parenthesis */
		else if (lexer->pos.cc == '(') {

			if (!em_lexer_add_token(lexer, EM_TOKEN_TYPE_OPEN_PAREN, &lexer->pos, "("))
				return EM_RESULT_FAILURE;
			em_pos_advance(&lexer->pos);
		}

		/* closing parenthesis */
		else if (lexer->pos.cc == ')') {

			if (!em_lexer_add_token(lexer, EM_TOKEN_TYPE_CLOSE_PAREN, &lexer->pos, ")"))
				return EM_RESULT_FAILURE;
			em_pos_advance(&lexer->pos);
		}

		/* open square bracket */
		else if (lexer->pos.cc == '[') {

			if (!em_lexer_add_token(lexer, EM_TOKEN_TYPE_OPEN_SQUARE_BRACKET, &lexer->pos, "["))
				return EM_RESULT_FAILURE;
			em_pos_advance(&lexer->pos);
		}

		/* closing square bracket */
		else if (lexer->pos.cc == ']') {

			if (!em_lexer_add_token(lexer, EM_TOKEN_TYPE_CLOSE_SQUARE_BRACKET, &lexer->pos, "]"))
				return EM_RESULT_FAILURE;
			em_pos_advance(&lexer->pos);
		}

		/* comma */
		else if (lexer->pos.cc == ',') {

			if (!em_lexer_add_token(lexer, EM_TOKEN_TYPE_COMMA, &lexer->pos, ","))
				return EM_RESULT_FAILURE;
			em_pos_advance(&lexer->pos);
		}

		/* dot */
		else if (lexer->pos.cc == '.') {

			if (!em_lexer_add_token(lexer, EM_TOKEN_TYPE_DOT, &lexer->pos, "."))
				return EM_RESULT_FAILURE;
			em_pos_advance(&lexer->pos);
		}

		/* equals */
		else if (lexer->pos.cc == '=') {

			em_pos_t pos = lexer->pos;
			em_pos_advance(&lexer->pos);

			em_token_type_t type = EM_TOKEN_TYPE_EQUALS;
			const char *value = "=";

			if (lexer->pos.cc == '=') {

				em_pos_advance(&lexer->pos);
				type = EM_TOKEN_TYPE_DOUBLE_EQUALS;
				value = "==";
			}

			if (!em_lexer_add_token(lexer, type, &pos, value))
				return EM_RESULT_FAILURE;
		}

		/* less than */
		else if (lexer->pos.cc == '<') {

			em_pos_t pos = lexer->pos;
			em_pos_advance(&lexer->pos);

			em_token_type_t type = EM_TOKEN_TYPE_LESS_THAN;
			const char *value = "<";

			if (lexer->pos.cc == '=') {

				em_pos_advance(&lexer->pos);
				type = EM_TOKEN_TYPE_LESS_THAN_EQUALS;
				value = "<=";
			}
			else if (lexer->pos.cc == '<') {

				em_pos_advance(&lexer->pos);
				type = EM_TOKEN_TYPE_BITWISE_LEFT_SHIFT;
				value = "<<";
			}

			if (!em_lexer_add_token(lexer, type, &pos, value))
				return EM_RESULT_FAILURE;
		}

		/* greater than */
		else if (lexer->pos.cc == '>') {

			em_pos_t pos = lexer->pos;
			em_pos_advance(&lexer->pos);

			em_token_type_t type = EM_TOKEN_TYPE_GREATER_THAN;
			const char *value = ">";

			if (lexer->pos.cc == '=') {

				em_pos_advance(&lexer->pos);
				type = EM_TOKEN_TYPE_GREATER_THAN_EQUALS;
				value = ">=";
			}
			else if (lexer->pos.cc == '>') {

				em_pos_advance(&lexer->pos);
				type = EM_TOKEN_TYPE_BITWISE_RIGHT_SHIFT;
				value = ">>";
			}

			if (!em_lexer_add_token(lexer, type, &pos, value))
				return EM_RESULT_FAILURE;
		}

		/* not equals */
		else if (lexer->pos.cc == '!') {

			em_pos_t pos = lexer->pos;
			em_pos_advance(&lexer->pos);

			if (lexer->pos.cc != '=') {

				em_log_error(&lexer->pos, "Expected '='");
				return EM_RESULT_FAILURE;
			}
			em_pos_advance(&lexer->pos);

			if (!em_lexer_add_token(lexer, EM_TOKEN_TYPE_NOT_EQUALS, &pos, "!="))
				return EM_RESULT_FAILURE;
		}

		/* bitwise and */
		else if (lexer->pos.cc == '&') {

			if (!em_lexer_add_token(lexer, EM_TOKEN_TYPE_BITWISE_AND, &lexer->pos, "&"))
				return EM_RESULT_FAILURE;
			em_pos_advance(&lexer->pos);
		}

		/* bitwise or */
		else if (lexer->pos.cc == '|') {

			if (!em_lexer_add_token(lexer, EM_TOKEN_TYPE_BITWISE_OR, &lexer->pos, "|"))
				return EM_RESULT_FAILURE;
			em_pos_advance(&lexer->pos);
		}

		/* bitwise not */
		else if (lexer->pos.cc == '~') {

			if (!em_lexer_add_token(lexer, EM_TOKEN_TYPE_BITWISE_NOT, &lexer->pos, "~"))
				return EM_RESULT_FAILURE;
			em_pos_advance(&lexer->pos);
		}

		/* unrecognized */
		else {

			char cc[5];
			em_ssize_t size = em_utf8_putch(cc, lexer->pos.cc);
			if (size < 0)
				em_log_error(&lexer->pos, "Unrecognized character");
			else {
				
				cc[size] = 0;
				em_log_error(&lexer->pos, "Unrecognized character '%s'", cc);
			}
			return EM_RESULT_FAILURE;
		}
	}

	if (!em_lexer_add_token(lexer, EM_TOKEN_TYPE_EOF, &lexer->pos, ""))
		return EM_RESULT_FAILURE;
	return EM_RESULT_SUCCESS;
}

/* make numeric token */
EM_API em_result_t em_lexer_make_number(em_lexer_t *lexer) {

	em_pos_t pos = lexer->pos;
	size_t len = 0;
	em_token_type_t type = EM_TOKEN_TYPE_INT;

	while (ISDIGIT(lexer->pos.cc) || lexer->pos.cc == '.') {

		if (lexer->pos.cc == '.') {
			if (type == EM_TOKEN_TYPE_FLOAT)
				break;
			else type = EM_TOKEN_TYPE_FLOAT;
		}

		em_ssize_t chlen = em_utf8_getchlen(lexer->pos.cc);
		if (chlen < 1 || chlen > 4) {

			em_log_error(&lexer->pos, "Invalid UTF-8 ordinal %d\n", lexer->pos.cc);
			return EM_RESULT_FAILURE;
		}
		len += (size_t)chlen;

		em_pos_advance(&lexer->pos);
	}

	if (!em_lexer_add_token_full(lexer, type, &pos, pos.text+pos.index, len))
		return EM_RESULT_FAILURE;
	return EM_RESULT_SUCCESS;
}

/* make identifier or keyword token */
EM_API em_result_t em_lexer_make_identifier(em_lexer_t *lexer) {

	em_pos_t pos = lexer->pos;
	size_t len = 0;

	while (ISIDENT_A(lexer->pos.cc)) {

		em_ssize_t chlen = em_utf8_getchlen(lexer->pos.cc);
		if (chlen < 1 || chlen > 4) {

			em_log_error(&lexer->pos, "Invalid UTF-8 ordinal %d\n", lexer->pos.cc);
			return EM_RESULT_FAILURE;
		}
		len += (size_t)chlen;

		em_pos_advance(&lexer->pos);
	}

	em_token_t *token;
	if (!(token = em_lexer_add_token_full(lexer, EM_TOKEN_TYPE_IDENTIFIER, &pos, pos.text+pos.index, len)))
		return EM_RESULT_FAILURE;

	/* identify a keyword */
	for (int i = 0; i < N_KEYWORDS; i++) {
		if (!strcmp(keywords[i], token->value)) {

			token->type = EM_TOKEN_TYPE_KEYWORD;
			break;
		}
	}
	return EM_RESULT_SUCCESS;
}

/* make string token */
EM_API em_result_t em_lexer_make_string(em_lexer_t *lexer) {

	em_pos_t pos = lexer->pos;
	size_t len = 0;

	int delim = lexer->pos.cc;
	em_pos_advance(&lexer->pos);
	em_pos_t vstart = lexer->pos; /* value start position */

	while (lexer->pos.cc && lexer->pos.cc != delim) {

		em_ssize_t chlen;

		/* escape character */
		if (lexer->pos.cc == '\\') {

			em_pos_advance(&lexer->pos);
			chlen = em_utf8_getchlen(getescchar(lexer->pos.cc));
		}
		else chlen = em_utf8_getchlen(lexer->pos.cc);

		if (chlen < 1 || chlen > 4) {

			em_log_error(&lexer->pos, "Invalid UTF-8 ordinal %d\n", lexer->pos.cc);
			return EM_RESULT_FAILURE;
		}
		len += (size_t)chlen;

		em_pos_advance(&lexer->pos);
	}

	/* expected delimeter */
	if (lexer->pos.cc != delim) {

		em_log_error(&lexer->pos, "Unexpected end of file");
		return EM_RESULT_FAILURE;
	}
	em_pos_advance(&lexer->pos);

	/* create token value */
	em_token_t *token;
	if (!(token = em_lexer_add_token_full(lexer, EM_TOKEN_TYPE_STRING, &pos, NULL, len)))
		return EM_RESULT_FAILURE;

	size_t i = 0;
	while (i < len) {

		int ch = vstart.cc;
		if (ch == '\\') {

			em_pos_advance(&vstart);
			ch = getescchar(vstart.cc);
		}

		/* get utf-8 character length */
		em_ssize_t chlen = em_utf8_getchlen(ch);
		if (chlen < 1 || chlen > 4) {

			em_log_error(&vstart, "Invalid UTF-8 ordinal %d\n", ch);
			return EM_RESULT_FAILURE;
		}
		if (i+(size_t)chlen > len) break;

		em_utf8_putch(token->value+i, ch);
		em_pos_advance(&vstart);
		i += (size_t)chlen;
	}
	return EM_RESULT_SUCCESS;
}

/* destroy lexer */
EM_API void em_lexer_destroy(em_lexer_t *lexer) {

	if (!lexer || !lexer->init) return;

	em_token_t *cur = lexer->first;
	while (cur) {

		em_token_t *next = cur->next;

		cur->next = NULL;
		em_refobj_decref(EM_REFOBJ(cur));

		cur = cur->next;
	}

	lexer->init = EM_FALSE;
}
