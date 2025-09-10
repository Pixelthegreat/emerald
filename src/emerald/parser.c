/*
 * Copyright 2025, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <emerald/core.h>
#include <emerald/memory.h>
#include <emerald/parser.h>

/* check if token is in list of match pairs */
static em_bool_t is_token_in(em_token_t *token, em_token_pair_t *pairs, size_t npairs) {

	for (size_t i = 0; i < npairs; i++) {

		if (pairs[i].value) {
			if (em_token_matches(token, pairs[i].type, pairs[i].value))
				return EM_TRUE;
		}
		else if (token->type == pairs[i].type)
			return EM_TRUE;
	}
	return EM_FALSE;
}

/* initialize parser */
EM_API em_result_t em_parser_init(em_parser_t *parser) {

	if (!parser || parser->init)
		return EM_RESULT_FAILURE;

	parser->token = NULL;
	parser->node = NULL;
	parser->init = EM_TRUE;

	return EM_RESULT_SUCCESS;
}

/* reset parser */
EM_API void em_parser_reset(em_parser_t *parser, em_token_t *token) {

	parser->token = token;
	if (parser->node) EM_NODE_DECREF(parser->node);

	parser->node = NULL;
}

/* advance parser */
EM_API void em_parser_advance(em_parser_t *parser) {

	if (!parser->token) return;

	parser->token = parser->token->next;
}

/* parse tokens */
EM_API em_result_t em_parser_parse(em_parser_t *parser) {

	parser->node = em_node_new(EM_NODE_TYPE_BLOCK, &parser->token->pos);

	while (parser->token->type != EM_TOKEN_TYPE_EOF) {

		em_node_t *statement = em_parser_statement(parser);
		if (!statement) return EM_RESULT_FAILURE;

		em_node_add_child(parser->node, statement);
	}
	return EM_RESULT_SUCCESS;
}

/* generic statement */
EM_API em_node_t *em_parser_statement(em_parser_t *parser) {

	em_token_t *token = parser->token;

	/* continue */
	if (em_token_matches(token, EM_TOKEN_TYPE_KEYWORD, "continue")) {

		em_parser_advance(parser);
		return em_node_new(EM_NODE_TYPE_CONTINUE, &token->pos);
	}

	/* break */
	else if (em_token_matches(token, EM_TOKEN_TYPE_KEYWORD, "break")) {

		em_parser_advance(parser);
		return em_node_new(EM_NODE_TYPE_BREAK, &token->pos);
	}

	/* return value */
	else if (em_token_matches(token, EM_TOKEN_TYPE_KEYWORD, "return")) {

		em_parser_advance(parser);

		em_node_t *expr = em_parser_expr(parser);
		if (!expr) return NULL;

		em_node_t *node = em_node_new(EM_NODE_TYPE_RETURN, &token->pos);
		em_node_add_child(node, expr);

		return node;
	}

	/* raise error */
	else if (em_token_matches(token, EM_TOKEN_TYPE_KEYWORD, "raise")) {

		em_parser_advance(parser);

		em_node_t *expr = em_parser_expr(parser);
		if (!expr) return NULL;

		em_node_t *node = em_node_new(EM_NODE_TYPE_RAISE, &token->pos);
		em_node_add_child(node, expr);

		return node;
	}

	/* include file */
	else if (em_token_matches(token, EM_TOKEN_TYPE_KEYWORD, "include")) {

		em_parser_advance(parser);

		em_node_t *expr = em_parser_expr(parser);
		if (!expr) return NULL;

		em_node_t *node = em_node_new(EM_NODE_TYPE_INCLUDE, &token->pos);
		em_node_add_child(node, expr);

		return node;
	}

	/* otherwise */
	else return em_parser_expr(parser);
}

/* binary operation */
EM_API em_node_t *em_parser_binop(em_parser_t *parser, em_node_t *(*func)(em_parser_t *), em_token_pair_t *pairs, size_t npairs) {

	em_node_t *left = func(parser);
	if (!left) return NULL;

	while (is_token_in(parser->token, pairs, npairs)) {

		em_token_t *op = parser->token;
		em_parser_advance(parser);

		em_node_t *right = func(parser);
		if (!right) { EM_NODE_DECREF(left); return NULL; }

		em_node_t *new = em_node_new(EM_NODE_TYPE_BINARY_OPERATION, &left->pos);
		em_node_add_child(new, left);
		em_node_add_token(new, op);
		em_node_add_child(new, right);

		left = new;
	}
	return left;
}

/* expression */
EM_API em_node_t *em_parser_expr(em_parser_t *parser) {

	em_token_pair_t pairs[] = {
		{EM_TOKEN_TYPE_KEYWORD, "and"},
		{EM_TOKEN_TYPE_KEYWORD, "or"},
	};
	return em_parser_binop(parser, em_parser_comp_expr, pairs, EM_TOKEN_PAIR_COUNT(pairs));
}

/* comparison expression */
EM_API em_node_t *em_parser_comp_expr(em_parser_t *parser) {

	em_token_pair_t pairs[] = {
		{EM_TOKEN_TYPE_DOUBLE_EQUALS, NULL}, {EM_TOKEN_TYPE_NOT_EQUALS, NULL},
		{EM_TOKEN_TYPE_LESS_THAN, NULL}, {EM_TOKEN_TYPE_GREATER_THAN, NULL},
		{EM_TOKEN_TYPE_LESS_THAN_EQUALS, NULL}, {EM_TOKEN_TYPE_GREATER_THAN_EQUALS, NULL},
	};
	return em_parser_binop(parser, em_parser_arith_expr, pairs, EM_TOKEN_PAIR_COUNT(pairs));
}

/* arithmetic expression */
EM_API em_node_t *em_parser_arith_expr(em_parser_t *parser) {

	em_token_pair_t pairs[] = {
		{EM_TOKEN_TYPE_PLUS, NULL}, {EM_TOKEN_TYPE_MINUS, NULL},
		{EM_TOKEN_TYPE_BITWISE_OR, NULL}, {EM_TOKEN_TYPE_BITWISE_AND, NULL},
	};
	return em_parser_binop(parser, em_parser_term, pairs, EM_TOKEN_PAIR_COUNT(pairs));
}

/* term */
EM_API em_node_t *em_parser_term(em_parser_t *parser) {

	em_token_pair_t pairs[] = {
		{EM_TOKEN_TYPE_ASTERISK, NULL}, {EM_TOKEN_TYPE_SLASH, NULL},
		{EM_TOKEN_TYPE_BITWISE_RIGHT_SHIFT, NULL}, {EM_TOKEN_TYPE_BITWISE_LEFT_SHIFT, NULL},
		{EM_TOKEN_TYPE_MODULO, NULL},
	};
	return em_parser_binop(parser, em_parser_call, pairs, EM_TOKEN_PAIR_COUNT(pairs));
}

/* call */
EM_API em_node_t *em_parser_call(em_parser_t *parser) {

	em_node_t *factor = em_parser_factor(parser);
	if (!factor) return NULL;

	em_node_t *prev = NULL;
	em_node_t *next = factor;
	while (next) {

		em_bool_t error = EM_FALSE;
		em_node_t *new = em_parser_call_extension(parser, next, &error);

		if (!new && error) {
			
			EM_NODE_DECREF(next);
			return NULL;
		}
		prev = next;
		next = new;
	}
	return prev;
}

/* call extension */
EM_API em_node_t *em_parser_call_extension(em_parser_t *parser, em_node_t *factor, em_bool_t *error) {

	*error = EM_FALSE;

	/* function call */
	if (parser->token->type == EM_TOKEN_TYPE_OPEN_PAREN) {

		em_parser_advance(parser);

		em_node_t *node = em_node_new(EM_NODE_TYPE_CALL, &factor->pos);
		em_node_add_child(node, factor);

		/* arguments */
		if (parser->token->type != EM_TOKEN_TYPE_CLOSE_PAREN) {

			em_node_t *expr = em_parser_expr(parser);
			if (!expr) {

				*error = EM_TRUE;
				EM_NODE_DECREF(node);
				return NULL;
			}
			em_node_add_child(node, expr);

			while (parser->token->type == EM_TOKEN_TYPE_COMMA) {

				em_parser_advance(parser);

				expr = em_parser_expr(parser);
				if (!expr) {

					*error = EM_TRUE;
					EM_NODE_DECREF(node);
					return NULL;
				}
				em_node_add_child(node, expr);
			}
		}

		/* end */
		if (parser->token->type != EM_TOKEN_TYPE_CLOSE_PAREN) {

			em_log_syntax_error(&parser->token->pos, "Expected ')'");
			*error = EM_TRUE;
			EM_NODE_DECREF(node);
			return NULL;
		}
		em_parser_advance(parser);

		return node;
	}

	/* member access */
	else if (parser->token->type == EM_TOKEN_TYPE_DOT) {

		em_parser_advance(parser);
		if (parser->token->type != EM_TOKEN_TYPE_IDENTIFIER) {

			em_log_syntax_error(&parser->token->pos, "Expected member name after '.'");
			*error = EM_TRUE;
			return NULL;
		}
		em_token_t *name = parser->token;
		em_parser_advance(parser);

		em_node_t *node = em_node_new(EM_NODE_TYPE_ACCESS, &factor->pos);
		em_node_add_child(node, factor);
		em_node_add_token(node, name);

		return node;
	}

	/* list access */
	else if (parser->token->type == EM_TOKEN_TYPE_OPEN_SQUARE_BRACKET) {

		em_parser_advance(parser);
		em_node_t *expr = em_parser_expr(parser);
		if (!expr) {

			*error = EM_TRUE;
			return NULL;
		}
		if (parser->token->type != EM_TOKEN_TYPE_CLOSE_SQUARE_BRACKET) {

			em_log_syntax_error(&parser->token->pos, "Expected ']'");
			*error = EM_TRUE;
			return NULL;
		}
		em_parser_advance(parser);

		em_node_t *node = em_node_new(EM_NODE_TYPE_ACCESS, &factor->pos);
		em_node_add_child(node, factor);
		em_node_add_child(node, expr);

		return node;
	}

	/* otherwise */
	else return NULL;
}

/* factor */
EM_API em_node_t *em_parser_factor(em_parser_t *parser) {

	em_token_t *token = parser->token;

	/* unary operation */
	em_token_pair_t pairs[] = {
		{EM_TOKEN_TYPE_PLUS, NULL}, {EM_TOKEN_TYPE_MINUS, NULL},
		{EM_TOKEN_TYPE_BITWISE_NOT, NULL}, {EM_TOKEN_TYPE_KEYWORD, "not"},
	};
	if (is_token_in(token, pairs, EM_TOKEN_PAIR_COUNT(pairs))) {

		em_parser_advance(parser);

		em_node_t *factor;
		if (token->type == EM_TOKEN_TYPE_KEYWORD)
			factor = em_parser_comp_expr(parser);
		else factor = em_parser_factor(parser);

		if (!factor) return NULL;

		em_node_t *node = em_node_new(EM_NODE_TYPE_UNARY_OPERATION, &token->pos);
		em_node_add_token(node, token);
		em_node_add_child(node, factor);

		return node;
	}

	/* grouped expression */
	else if (token->type == EM_TOKEN_TYPE_OPEN_PAREN) {

		em_parser_advance(parser);

		em_node_t *expr = em_parser_expr(parser);
		if (!expr) return NULL;

		if (parser->token->type != EM_TOKEN_TYPE_CLOSE_PAREN) {

			em_log_syntax_error(&parser->token->pos, "Expected ')'");
			return NULL;
		}
		em_parser_advance(parser);

		return expr;
	}

	/* list initializer */
	else if (token->type == EM_TOKEN_TYPE_OPEN_SQUARE_BRACKET) {

		em_parser_advance(parser);

		em_node_t *node = em_node_new(EM_NODE_TYPE_LIST, &token->pos);
		if (parser->token->type != EM_TOKEN_TYPE_CLOSE_SQUARE_BRACKET) {

			em_node_t *expr = em_parser_expr(parser);
			if (!expr) { EM_NODE_DECREF(node); return NULL; }
			em_node_add_child(node, expr);

			while (parser->token->type == EM_TOKEN_TYPE_COMMA) {

				em_parser_advance(parser);
				if (parser->token->type == EM_TOKEN_TYPE_CLOSE_SQUARE_BRACKET)
					break;

				expr = em_parser_expr(parser);
				if (!expr) { EM_NODE_DECREF(node); return NULL; }
				em_node_add_child(node, expr);
			}
		}

		/* end */
		if (parser->token->type != EM_TOKEN_TYPE_CLOSE_SQUARE_BRACKET) {

			em_log_syntax_error(&parser->token->pos, "Expected ']'");
			EM_NODE_DECREF(node);
			return NULL;
		}
		em_parser_advance(parser);
		return node;
	}

	/* map initializer */
	else if (token->type == EM_TOKEN_TYPE_OPEN_BRACKET) {

		em_parser_advance(parser);

		em_node_t *node = em_node_new(EM_NODE_TYPE_MAP, &token->pos);
		if (parser->token->type != EM_TOKEN_TYPE_CLOSE_BRACKET) {

			em_node_t *expr = em_parser_expr(parser);
			if (!expr) { EM_NODE_DECREF(node); return NULL; }
			em_node_add_child(node, expr);

			if (parser->token->type != EM_TOKEN_TYPE_COLON) {

				em_log_syntax_error(&parser->token->pos, "Expected ':'");
				EM_NODE_DECREF(node);
				return NULL;
			}
			em_parser_advance(parser);

			expr = em_parser_expr(parser);
			if (!expr) { EM_NODE_DECREF(node); return NULL; }
			em_node_add_child(node, expr);

			/* more */
			while (parser->token->type == EM_TOKEN_TYPE_COMMA) {

				em_parser_advance(parser);
				if (parser->token->type == EM_TOKEN_TYPE_CLOSE_BRACKET)
					break;

				expr = em_parser_expr(parser);
				if (!expr) { EM_NODE_DECREF(node); return NULL; }
				em_node_add_child(node, expr);

				if (parser->token->type != EM_TOKEN_TYPE_COLON) {

					em_log_syntax_error(&parser->token->pos, "Expected ':'");
					EM_NODE_DECREF(node);
					return NULL;
				}
				em_parser_advance(parser);

				expr = em_parser_expr(parser);
				if (!expr) { EM_NODE_DECREF(node); return NULL; }
				em_node_add_child(node, expr);
			}
		}

		/* end */
		if (parser->token->type != EM_TOKEN_TYPE_CLOSE_BRACKET) {

			em_log_syntax_error(&parser->token->pos, "Expected '}'");
			EM_NODE_DECREF(node);
			return NULL;
		}
		em_parser_advance(parser);

		return node;
	}

	/* integer */
	else if (token->type == EM_TOKEN_TYPE_INT) {

		em_parser_advance(parser);

		em_node_t *node = em_node_new(EM_NODE_TYPE_INT, &token->pos);
		em_node_add_token(node, token);

		return node;
	}

	/* float */
	else if (token->type == EM_TOKEN_TYPE_FLOAT) {

		em_parser_advance(parser);

		em_node_t *node = em_node_new(EM_NODE_TYPE_FLOAT, &token->pos);
		em_node_add_token(node, token);

		return node;
	}

	/* string */
	else if (token->type == EM_TOKEN_TYPE_STRING) {

		em_parser_advance(parser);

		em_node_t *node = em_node_new(EM_NODE_TYPE_STRING, &token->pos);
		em_node_add_token(node, token);

		return node;
	}

	/* identifier */
	else if (token->type == EM_TOKEN_TYPE_IDENTIFIER) {

		em_parser_advance(parser);

		em_node_t *node = em_node_new(EM_NODE_TYPE_IDENTIFIER, &token->pos);
		em_node_add_token(node, token);

		return node;
	}

	/* variable definition */
	else if (em_token_matches(token, EM_TOKEN_TYPE_KEYWORD, "let"))
		return em_parser_let_statement(parser);

	/* function definition */
	else if (em_token_matches(token, EM_TOKEN_TYPE_KEYWORD, "func"))
		return em_parser_func_statement(parser);

	/* class definition */
	else if (em_token_matches(token, EM_TOKEN_TYPE_KEYWORD, "class"))
		return em_parser_class_statement(parser);

	/* try catch block */
	else if (em_token_matches(token, EM_TOKEN_TYPE_KEYWORD, "try"))
		return em_parser_try_statement(parser);

	/* if statement */
	else if (em_token_matches(token, EM_TOKEN_TYPE_KEYWORD, "if")) {

		em_parser_advance(parser);

		em_node_t *expr = em_parser_expr(parser);
		if (!expr) return NULL;

		em_node_t *node = em_node_new(EM_NODE_TYPE_IF, &token->pos);
		em_node_add_child(node, expr);

		if (!em_token_matches(parser->token, EM_TOKEN_TYPE_KEYWORD, "then")) {

			em_log_syntax_error(&parser->token->pos, "Expected 'then'");
			EM_NODE_DECREF(node);
			return NULL;
		}
		em_parser_advance(parser);

		/* main body */
		em_node_t *block = em_node_new(EM_NODE_TYPE_BLOCK, &parser->token->pos);
		em_node_add_child(node, block);

		em_token_pair_t pairs[] = {
			{EM_TOKEN_TYPE_KEYWORD, "elif"}, {EM_TOKEN_TYPE_KEYWORD, "else"},
			{EM_TOKEN_TYPE_KEYWORD, "end"},
		};

		while (!is_token_in(parser->token, pairs, EM_TOKEN_PAIR_COUNT(pairs))) {

			em_node_t *statement = em_parser_statement(parser);
			if (!statement) { EM_NODE_DECREF(node); return NULL; }
			em_node_add_child(block, statement);
		}

		/* elif */
		while (em_token_matches(parser->token, EM_TOKEN_TYPE_KEYWORD, "elif")) {

			em_parser_advance(parser);

			expr = em_parser_expr(parser);
			if (!expr) { EM_NODE_DECREF(node); return NULL; }
			em_node_add_child(node, expr);

			if (!em_token_matches(parser->token, EM_TOKEN_TYPE_KEYWORD, "then")) {

				em_log_syntax_error(&parser->token->pos, "Expected 'then'");
				EM_NODE_DECREF(node);
				return NULL;
			}
			em_parser_advance(parser);

			block = em_node_new(EM_NODE_TYPE_BLOCK, &parser->token->pos);
			em_node_add_child(node, block);

			while (!is_token_in(parser->token, pairs+1, EM_TOKEN_PAIR_COUNT(pairs)-1)) {

				em_node_t *statement = em_parser_statement(parser);
				if (!statement) { EM_NODE_DECREF(node); return NULL; }
				em_node_add_child(block, statement);
			}
		}

		/* else */
		if (em_token_matches(parser->token, EM_TOKEN_TYPE_KEYWORD, "else")) {

			em_parser_advance(parser);
			if (!em_token_matches(parser->token, EM_TOKEN_TYPE_KEYWORD, "then")) {

				em_log_syntax_error(&parser->token->pos, "Expected 'then'");
				EM_NODE_DECREF(node);
				return NULL;
			}
			em_parser_advance(parser);

			block = em_node_new(EM_NODE_TYPE_BLOCK, &parser->token->pos);
			em_node_add_child(node, block);

			while (!em_token_matches(parser->token, EM_TOKEN_TYPE_KEYWORD, "end")) {

				em_node_t *statement = em_parser_statement(parser);
				if (!statement) { EM_NODE_DECREF(node); return NULL; }
				em_node_add_child(block, statement);
			}
		}

		/* end */
		if (!em_token_matches(parser->token, EM_TOKEN_TYPE_KEYWORD, "end")) {

			em_log_syntax_error(&parser->token->pos, "Expected 'end'");
			EM_NODE_DECREF(node);
			return NULL;
		}
		em_parser_advance(parser);

		return node;
	}

	/* for loop */
	else if (em_token_matches(token, EM_TOKEN_TYPE_KEYWORD, "for")) {

		em_parser_advance(parser);

		if (parser->token->type != EM_TOKEN_TYPE_IDENTIFIER) {

			em_log_syntax_error(&parser->token->pos, "Expected iterator name");
			return NULL;
		}
		em_token_t *name = parser->token;
		em_parser_advance(parser);

		em_node_t *node = em_node_new(EM_NODE_TYPE_FOR, &token->pos);
		em_node_add_token(node, name);

		if (parser->token->type != EM_TOKEN_TYPE_EQUALS) {

			em_log_syntax_error(&parser->token->pos, "Expected '='");
			EM_NODE_DECREF(node);
			return NULL;
		}
		em_parser_advance(parser);

		em_node_t *start = em_parser_expr(parser);
		if (!start) { EM_NODE_DECREF(node); return NULL; }
		em_node_add_child(node, start);

		if (!em_token_matches(parser->token, EM_TOKEN_TYPE_KEYWORD, "to")) {

			em_log_syntax_error(&parser->token->pos, "Expected 'to'");
			EM_NODE_DECREF(node);
			return NULL;
		}
		em_parser_advance(parser);

		em_node_t *end = em_parser_expr(parser);
		if (!end) { EM_NODE_DECREF(node); return NULL; }
		em_node_add_child(node, end);

		if (!em_token_matches(parser->token, EM_TOKEN_TYPE_KEYWORD, "then")) {

			em_log_syntax_error(&parser->token->pos, "Expected 'then'");
			EM_NODE_DECREF(node);
			return NULL;
		}
		em_parser_advance(parser);

		/* loop body */
		em_node_t *block = em_node_new(EM_NODE_TYPE_BLOCK, &parser->token->pos);
		em_node_add_child(node, block);

		while (!em_token_matches(parser->token, EM_TOKEN_TYPE_KEYWORD, "end")) {

			em_node_t *statement = em_parser_statement(parser);
			if (!statement) { EM_NODE_DECREF(node); return NULL; }
			em_node_add_child(block, statement);
		}
		if (!em_token_matches(parser->token, EM_TOKEN_TYPE_KEYWORD, "end")) {

			em_log_syntax_error(&parser->token->pos, "Expected 'end'");
			EM_NODE_DECREF(node);
			return NULL;
		}
		em_parser_advance(parser);

		return node;
	}

	/* foreach loop */
	else if (em_token_matches(token, EM_TOKEN_TYPE_KEYWORD, "foreach")) {

		em_parser_advance(parser);
		if (parser->token->type != EM_TOKEN_TYPE_IDENTIFIER) {

			em_log_syntax_error(&parser->token->pos, "Expected iterator name");
			return NULL;
		}
		em_token_t *name = parser->token;
		em_parser_advance(parser);

		if (!em_token_matches(parser->token, EM_TOKEN_TYPE_KEYWORD, "in")) {

			em_log_syntax_error(&parser->token->pos, "Expected 'in'");
			return NULL;
		}
		em_parser_advance(parser);

		em_node_t *expr = em_parser_expr(parser);
		if (!expr) return NULL;

		em_node_t *node = em_node_new(EM_NODE_TYPE_FOREACH, &token->pos);
		em_node_add_token(node, name);
		em_node_add_child(node, expr);

		/* body */
		if (!em_token_matches(parser->token, EM_TOKEN_TYPE_KEYWORD, "then")) {

			em_log_syntax_error(&parser->token->pos, "Expected 'then'");
			EM_NODE_DECREF(node);
			return NULL;
		}
		em_parser_advance(parser);

		em_node_t *block = em_node_new(EM_NODE_TYPE_BLOCK, &parser->token->pos);
		em_node_add_child(node, block);

		while (!em_token_matches(parser->token, EM_TOKEN_TYPE_KEYWORD, "end")) {

			em_node_t *statement = em_parser_statement(parser);
			if (!statement) { EM_NODE_DECREF(node); return NULL; }
			em_node_add_child(block, statement);
		}
		if (!em_token_matches(parser->token, EM_TOKEN_TYPE_KEYWORD, "end")) {

			em_log_syntax_error(&parser->token->pos, "Expected 'end'");
			EM_NODE_DECREF(node);
			return NULL;
		}
		em_parser_advance(parser);

		return node;
	}

	/* while loop */
	if (em_token_matches(token, EM_TOKEN_TYPE_KEYWORD, "while")) {

		em_parser_advance(parser);

		em_node_t *expr = em_parser_expr(parser);
		if (!expr) return NULL;

		em_node_t *node = em_node_new(EM_NODE_TYPE_WHILE, &token->pos);
		em_node_add_child(node, expr);

		if (!em_token_matches(parser->token, EM_TOKEN_TYPE_KEYWORD, "then")) {

			em_log_syntax_error(&parser->token->pos, "Expected 'then'");
			EM_NODE_DECREF(node);
			return NULL;
		}
		em_parser_advance(parser);

		em_node_t *block = em_node_new(EM_NODE_TYPE_BLOCK, &parser->token->pos);
		em_node_add_child(node, block);

		while (!em_token_matches(parser->token, EM_TOKEN_TYPE_KEYWORD, "end")) {

			em_node_t *statement = em_parser_statement(parser);
			if (!statement) { EM_NODE_DECREF(node); return NULL; }
			em_node_add_child(block, statement);
		}
		if (!em_token_matches(parser->token, EM_TOKEN_TYPE_KEYWORD, "end")) {

			em_log_syntax_error(&parser->token->pos, "Expected 'end'");
			EM_NODE_DECREF(node);
			return NULL;
		}
		em_parser_advance(parser);

		return node;
	}

	/* put string */
	else if (em_token_matches(token, EM_TOKEN_TYPE_KEYWORD, "puts")) {

		em_parser_advance(parser);

		em_node_t *expr = em_parser_expr(parser);
		if (!expr) return NULL;

		em_node_t *node = em_node_new(EM_NODE_TYPE_PUTS, &token->pos);
		em_node_add_child(node, expr);

		while (parser->token->type == EM_TOKEN_TYPE_COMMA) {

			em_parser_advance(parser);

			expr = em_parser_expr(parser);
			if (!expr) { EM_NODE_DECREF(node); return NULL; }
			em_node_add_child(node, expr);
		}

		return node;
	}

	/* end of file */
	else if (token->type == EM_TOKEN_TYPE_EOF) {

		em_log_syntax_error(&token->pos, "Unexpected end of file");
		return NULL;
	}

	/* other token */
	else {

		em_log_syntax_error(&token->pos, "Unexpected token '%s'", token->value);
		return NULL;
	}
}

/* variable definition */
EM_API em_node_t *em_parser_let_statement(em_parser_t *parser) {

	em_token_t *token = parser->token;

	em_parser_advance(parser);
	if (parser->token->type != EM_TOKEN_TYPE_IDENTIFIER) {

		em_log_syntax_error(&parser->token->pos, "Expected variable name");
		return NULL;
	}
	em_token_t *name = parser->token;
	em_parser_advance(parser);

	em_node_t *node = em_node_new(EM_NODE_TYPE_LET, &token->pos);
	em_node_add_token(node, name);

	while (parser->token->type == EM_TOKEN_TYPE_DOT) {

		em_parser_advance(parser);
		if (parser->token->type != EM_TOKEN_TYPE_IDENTIFIER) {

			em_log_syntax_error(&parser->token->pos, "Expected member name");
			EM_NODE_DECREF(node);
			return NULL;
		}
		em_node_add_token(node, parser->token);
		em_parser_advance(parser);
	}

	/* index */
	if (parser->token->type == EM_TOKEN_TYPE_OPEN_SQUARE_BRACKET) {

		em_parser_advance(parser);

		em_node_t *expr = em_parser_expr(parser);
		if (!expr) { EM_NODE_DECREF(node); return NULL; }
		em_node_add_child(node, expr);

		if (parser->token->type != EM_TOKEN_TYPE_CLOSE_SQUARE_BRACKET) {

			em_log_syntax_error(&parser->token->pos, "Expected ']'");
			EM_NODE_DECREF(node);
			return NULL;
		}
		em_parser_advance(parser);
	}

	/* value */
	if (parser->token->type != EM_TOKEN_TYPE_EQUALS) {

		em_log_syntax_error(&parser->token->pos, "Expected '='");
		EM_NODE_DECREF(node);
		return NULL;
	}
	em_parser_advance(parser);

	em_node_t *value = em_parser_expr(parser);
	if (!value) { EM_NODE_DECREF(node); return NULL; }

	em_node_add_child(node, value);
	return node;
}

/* function definition */
EM_API em_node_t *em_parser_func_statement(em_parser_t *parser) {

	em_token_t *token = parser->token;
	em_parser_advance(parser);

	em_token_t *name = NULL;
	if (parser->token->type == EM_TOKEN_TYPE_IDENTIFIER) {

		name = parser->token;
		em_parser_advance(parser);
	}

	if (parser->token->type != EM_TOKEN_TYPE_OPEN_PAREN) {

		em_log_syntax_error(&parser->token->pos, "Expected '('");
		return NULL;
	}
	em_parser_advance(parser);

	em_node_t *node = em_node_new(EM_NODE_TYPE_FUNC, &token->pos);
	if (name) {
		
		em_node_add_token(node, name);
		node->flags = 1;
	}

	if (parser->token->type != EM_TOKEN_TYPE_CLOSE_PAREN) {

		if (parser->token->type != EM_TOKEN_TYPE_IDENTIFIER) {

			em_log_syntax_error(&parser->token->pos, "Expected argument name");
			EM_NODE_DECREF(node);
			return NULL;
		}
		em_node_add_token(node, parser->token);
		em_parser_advance(parser);

		while (parser->token->type == EM_TOKEN_TYPE_COMMA) {

			em_parser_advance(parser);
			if (parser->token->type != EM_TOKEN_TYPE_IDENTIFIER) {

				em_log_syntax_error(&parser->token->pos, "Expected argument name");
				EM_NODE_DECREF(node);
				return NULL;
			}
			em_node_add_token(node, parser->token);
			em_parser_advance(parser);
		}
	}
	if (parser->token->type != EM_TOKEN_TYPE_CLOSE_PAREN) {

		em_log_syntax_error(&parser->token->pos, "Expected ')'");
		EM_NODE_DECREF(node);
		return NULL;
	}
	em_parser_advance(parser);

	/* body */
	if (!em_token_matches(parser->token, EM_TOKEN_TYPE_KEYWORD, "then")) {

		em_log_syntax_error(&parser->token->pos, "Expected 'then'");
		EM_NODE_DECREF(node);
		return NULL;
	}
	em_parser_advance(parser);

	em_node_t *block = em_node_new(EM_NODE_TYPE_BLOCK, &parser->token->pos);
	em_node_add_child(node, block);

	while (!em_token_matches(parser->token, EM_TOKEN_TYPE_KEYWORD, "end")) {

		em_node_t *statement = em_parser_statement(parser);
		if (!statement) { EM_NODE_DECREF(node); return NULL; }
		em_node_add_child(block, statement);
	}
	if (!em_token_matches(parser->token, EM_TOKEN_TYPE_KEYWORD, "end")) {

		em_log_syntax_error(&parser->token->pos, "Expected 'end'");
		EM_NODE_DECREF(node);
		return NULL;
	}
	em_parser_advance(parser);

	return node;
}

/* class definition */
EM_API em_node_t *em_parser_class_statement(em_parser_t *parser) {

	em_token_t *token = parser->token;
	em_parser_advance(parser);

	if (parser->token->type != EM_TOKEN_TYPE_IDENTIFIER) {

		em_log_syntax_error(&parser->token->pos, "Expected class name");
		return NULL;
	}
	em_token_t *name = parser->token;
	em_parser_advance(parser);

	em_node_t *node = em_node_new(EM_NODE_TYPE_CLASS, &token->pos);
	em_node_add_token(node, name);

	if (em_token_matches(parser->token, EM_TOKEN_TYPE_KEYWORD, "of")) {

		em_parser_advance(parser);

		em_node_t *expr = em_parser_expr(parser);
		if (!expr) { EM_NODE_DECREF(node); return NULL; }
		em_node_add_child(node, expr);
	}
	if (!em_token_matches(parser->token, EM_TOKEN_TYPE_KEYWORD, "then")) {

		em_log_syntax_error(&parser->token->pos, "Expected 'then'");
		EM_NODE_DECREF(node);
		return NULL;
	}
	em_parser_advance(parser);

	em_node_t *block = em_node_new(EM_NODE_TYPE_BLOCK, &parser->token->pos);
	em_node_add_child(node, block);

	while (!em_token_matches(parser->token, EM_TOKEN_TYPE_KEYWORD, "end")) {

		em_node_t *statement = em_parser_statement(parser);
		if (!statement) { EM_NODE_DECREF(node); return NULL; }
		em_node_add_child(block, statement);
	}
	if (!em_token_matches(parser->token, EM_TOKEN_TYPE_KEYWORD, "end")) {

		em_log_syntax_error(&parser->token->pos, "Expected 'end'");
		EM_NODE_DECREF(node);
		return NULL;
	}
	em_parser_advance(parser);

	return node;
}

/* try catch block */
EM_API em_node_t *em_parser_try_statement(em_parser_t *parser) {

	em_token_t *token = parser->token;
	em_parser_advance(parser);

	if (!em_token_matches(parser->token, EM_TOKEN_TYPE_KEYWORD, "then")) {

		em_log_syntax_error(&parser->token->pos, "Expected 'then'");
		return NULL;
	}
	em_parser_advance(parser);

	em_node_t *node = em_node_new(EM_NODE_TYPE_TRY, &token->pos);
	em_node_t *block = em_node_new(EM_NODE_TYPE_BLOCK, &parser->token->pos);
	em_node_add_child(node, block);

	while (!em_token_matches(parser->token, EM_TOKEN_TYPE_KEYWORD, "catch")) {

		em_node_t *statement = em_parser_statement(parser);
		if (!statement) { EM_NODE_DECREF(node); return NULL; }
		em_node_add_child(block, statement);
	}
	if (!em_token_matches(parser->token, EM_TOKEN_TYPE_KEYWORD, "catch")) {

		em_log_syntax_error(&parser->token->pos, "Expected 'catch'");
		EM_NODE_DECREF(node);
		return NULL;
	}
	em_parser_advance(parser);

	/* error type */
	if (parser->token->type == EM_TOKEN_TYPE_IDENTIFIER) {

		em_node_add_token(node, parser->token);
		em_parser_advance(parser);

		if (parser->token->type != EM_TOKEN_TYPE_EQUALS) {

			em_log_syntax_error(&parser->token->pos, "Expected '='");
			EM_NODE_DECREF(node);
			return NULL;
		}
		em_parser_advance(parser);

		em_node_t *expr = em_parser_expr(parser);
		if (!expr) { EM_NODE_DECREF(node); return NULL; }
		em_node_add_child(node, expr);
	}

	/* catch body */
	if (!em_token_matches(parser->token, EM_TOKEN_TYPE_KEYWORD, "then")) {

		em_log_syntax_error(&parser->token->pos, "Expected 'then'");
		EM_NODE_DECREF(node);
		return NULL;
	}
	em_parser_advance(parser);

	block = em_node_new(EM_NODE_TYPE_BLOCK, &parser->token->pos);
	em_node_add_child(node, block);

	while (!em_token_matches(parser->token, EM_TOKEN_TYPE_KEYWORD, "end")) {

		em_node_t *statement = em_parser_statement(parser);
		if (!statement) { EM_NODE_DECREF(node); return NULL; }
		em_node_add_child(block, statement);
	}
	if (!em_token_matches(parser->token, EM_TOKEN_TYPE_KEYWORD, "end")) {

		em_log_syntax_error(&parser->token->pos, "Expected 'end'");
		EM_NODE_DECREF(node);
		return NULL;
	}
	em_parser_advance(parser);

	return node;
}

/* destroy parser */
EM_API void em_parser_destroy(em_parser_t *parser) {

	if (!parser || !parser->init) return;

	if (parser->node) EM_NODE_DECREF(parser->node);
	parser->init = EM_FALSE;
}
