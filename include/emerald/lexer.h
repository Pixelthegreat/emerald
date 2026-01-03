/*
 * Copyright 2026, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef EMERALD_LEXER_H
#define EMERALD_LEXER_H

#include <emerald/core.h>
#include <emerald/token.h>

/* lexer */
typedef struct em_lexer {
	em_bool_t init; /* initialized */
	em_pos_t pos; /* position in file/text */
	em_pos_t initial_pos; /* initial position */
	em_token_t *first; /* first token */
	em_token_t *last; /* last token */
} em_lexer_t;

#define EM_LEXER_INIT ((em_lexer_t){EM_FALSE})

/* functions */
EM_API em_result_t em_lexer_init(em_lexer_t *lexer); /* initialize lexer */
EM_API void em_lexer_reset(em_lexer_t *lexer, const char *path, const char *text, em_ssize_t len); /* reset lexer */
EM_API em_token_t *em_lexer_add_token_full(em_lexer_t *lexer, em_token_type_t type, em_pos_t *pos, const char *value, size_t len); /* add token with length specified */
EM_API em_token_t *em_lexer_add_token(em_lexer_t *lexer, em_token_type_t type, em_pos_t *pos, const char *value); /* add token */
EM_API em_result_t em_lexer_make_tokens(em_lexer_t *lexer); /* generate tokens from input text */
EM_API em_result_t em_lexer_make_number(em_lexer_t *lexer); /* make numeric token */
EM_API em_result_t em_lexer_make_identifier(em_lexer_t *lexer); /* make identifier or keyword token */
EM_API em_result_t em_lexer_make_string(em_lexer_t *lexer); /* make string token */
EM_API void em_lexer_destroy(em_lexer_t *lexer); /* destroy lexer */

#endif /* EMERALD_LEXER_H */
