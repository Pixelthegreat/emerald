/*
 * Copyright 2026, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef EMERALD_NODE_H
#define EMERALD_NODE_H

#include <emerald/core.h>
#include <emerald/refobj.h>
#include <emerald/array.h>
#include <emerald/token.h>

/* node types */
typedef enum em_node_type {
	EM_NODE_TYPE_NONE = 0,

	/* simple/basic nodes */
	EM_NODE_TYPE_BLOCK, /* block of statements */
	EM_NODE_TYPE_INT, /* decimal number */
	EM_NODE_TYPE_FLOAT, /* floating point number */
	EM_NODE_TYPE_STRING, /* string of text */
	EM_NODE_TYPE_IDENTIFIER, /* variable or member name */
	EM_NODE_TYPE_LIST, /* list constructor */
	EM_NODE_TYPE_MAP, /* map constructor */

	EM_NODE_TYPE_UNARY_OPERATION, /* operation with one operator */
	EM_NODE_TYPE_BINARY_OPERATION, /* operation with two operators */

	EM_NODE_TYPE_ACCESS, /* member access */
	EM_NODE_TYPE_CALL, /* function call */

	/* keyword based nodes */
	EM_NODE_TYPE_CONTINUE, /* skip to next iteration of loop */
	EM_NODE_TYPE_BREAK, /* break out of loop */
	EM_NODE_TYPE_RETURN, /* return from function */
	EM_NODE_TYPE_RAISE, /* raise an exception */
	EM_NODE_TYPE_INCLUDE, /* include source file */

	EM_NODE_TYPE_LET, /* set variable */
	EM_NODE_TYPE_IF, /* run a block if a condition is met */
	EM_NODE_TYPE_FOR, /* loop for a fixed number of times */
	EM_NODE_TYPE_FOREACH, /* loop through an iterable object */
	EM_NODE_TYPE_WHILE, /* loop while a condition is met */
	EM_NODE_TYPE_FUNC, /* define a function */
	EM_NODE_TYPE_CLASS, /* define a class */
	EM_NODE_TYPE_TRY, /* run a block and catch any specified exceptions */
	EM_NODE_TYPE_PUTS, /* print objects to the console */

	EM_NODE_TYPE_COUNT,
} em_node_type_t;

/* node */
typedef struct em_node {
	em_refobj_t base;
	em_node_type_t type; /* type of node */
	em_pos_t pos; /* position */
	uint32_t flags; /* flag values */
	struct em_node *parent; /* parent */
	struct em_node *first; /* first child */
	struct em_node *last; /* last child */
	struct em_node *prev; /* previous sibling */
	struct em_node *next; /* next sibling */
	em_array_t tokens; /* saved tokens */
} em_node_t;

#define EM_NODE(p) ((em_node_t *)(p))

EM_API em_reflist_t em_reflist_node;

#define EM_NODE_INCREF(p) EM_NODE(em_refobj_incref(EM_REFOBJ(p)))
#define EM_NODE_DECREF(p) em_refobj_decref(EM_REFOBJ(p))

/* functions */
EM_API const char *em_get_node_type_name(em_node_type_t type); /* get name from type */

EM_API em_node_t *em_node_new(em_node_type_t type, em_pos_t *pos); /* create node */
EM_API void em_node_add_child(em_node_t *node, em_node_t *child); /* add child node */
EM_API void em_node_add_token(em_node_t *node, em_token_t *token); /* add token */
EM_API em_token_t *em_node_get_token(em_node_t *node, size_t index); /* get token */
EM_API void em_node_print(em_node_t *node); /* print node information */

#endif /* EMERALD_NODE_H */
