/*
 * Copyright 2026, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef EMERALD_PARSER_H
#define EMERALD_PARSER_H

#include <emerald/core.h>
#include <emerald/token.h>
#include <emerald/node.h>

/* parser */
typedef struct em_parser {
	em_bool_t init; /* initialized */
	em_token_t *token; /* current token */
	em_node_t *node; /* result node */
} em_parser_t;

#define EM_PARSER_INIT ((em_parser_t){EM_FALSE})

typedef struct em_token_pair {
	em_token_type_t type; /* type */
	const char *value; /* value */
} em_token_pair_t;

#define EM_TOKEN_PAIR_COUNT(pairs) (sizeof(pairs) / sizeof(pairs[0]))

/* functions */
EM_API em_result_t em_parser_init(em_parser_t *parser); /* initialize parser */
EM_API void em_parser_reset(em_parser_t *parser, em_token_t *token); /* reset parser */
EM_API void em_parser_advance(em_parser_t *parser); /* advance parser */
EM_API em_result_t em_parser_parse(em_parser_t *parser); /* parse tokens */
EM_API em_node_t *em_parser_statement(em_parser_t *parser); /* generic statement */
EM_API em_node_t *em_parser_binop(em_parser_t *parser, em_node_t *(*func)(em_parser_t *), em_token_pair_t *pairs, size_t npairs); /* binary operation */
EM_API em_node_t *em_parser_expr(em_parser_t *parser); /* expression */
EM_API em_node_t *em_parser_comp_expr(em_parser_t *parser); /* comparison expression */
EM_API em_node_t *em_parser_arith_expr(em_parser_t *parser); /* arithmetic expression */
EM_API em_node_t *em_parser_term(em_parser_t *parser); /* term */
EM_API em_node_t *em_parser_call(em_parser_t *parser); /* call */
EM_API em_node_t *em_parser_call_extension(em_parser_t *parser, em_node_t *factor, em_bool_t *error); /* call extension */
EM_API em_node_t *em_parser_factor(em_parser_t *parser); /* factor */
EM_API em_node_t *em_parser_let_statement(em_parser_t *parser); /* variable definition */
EM_API em_node_t *em_parser_func_statement(em_parser_t *parser); /* function definition */
EM_API em_node_t *em_parser_class_statement(em_parser_t *parser); /* class definition */
EM_API em_node_t *em_parser_try_statement(em_parser_t *parser); /* try catch block */
EM_API void em_parser_destroy(em_parser_t *parser); /* destroy parser */

#endif /* EMERALD_PARSER_H */
