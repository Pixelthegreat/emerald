/*
 * Copyright 2025, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <emerald/core.h>
#include <emerald/node.h>

/* node type names */
static const char *typenames[EM_NODE_TYPE_COUNT] = {
	"(None)",
	"BLOCK",
	"INT",
	"FLOAT",
	"STRING",
	"IDENTIFIER",
	"LIST",
	"MAP",
	"UNARY_OPERATION",
	"BINARY_OPERATION",
	"ACCESS",
	"CALL",
	"CONTINUE",
	"BREAK",
	"RETURN",
	"RAISE",
	"INCLUDE",
	"LET_STATEMENT",
	"IF_STATEMENT",
	"FOR_STATEMENT",
	"FOREACH_STATEMENT",
	"WHILE_STATEMENT",
	"FUNC_STATEMENT",
	"CLASS_STATEMENT",
	"TRY_STATEMENT",
	"PUTS_STATEMENT",
};

/* free callback */
static void node_free(void *p) {

	em_node_t *node = EM_NODE(p);

	if (node->parent) {
		 if (node->parent->first == node) node->parent->first = node->next;
		 if (node->parent->last == node) node->parent->last = node->prev;
	}
	if (node->prev) node->prev->next = node->next;
	if (node->next) node->next->prev = node->prev;

	/* unreference relatives */
	em_node_t *cur = node->first;
	while (cur) {

		em_node_t *next = cur->next;
		EM_NODE_DECREF(cur);
		cur = next;
	}
	
	/* unreference tokens */
	em_generic_result_t res = {.p = EM_TRUE};
	for (size_t i = 0; i < node->tokens.nitems && res.p; i++) {

		res = em_array_get(&node->tokens, i);
		if (res.p) EM_TOKEN_DECREF((em_token_t *)res.v.t_voidp);
	}
	em_array_destroy(&node->tokens);
}

/* get name from type */
EM_API const char *em_get_node_type_name(em_node_type_t type) {

	if (type < 1 || type >= EM_NODE_TYPE_COUNT)
		return NULL;
	return typenames[type];
}

/* create node */
EM_API em_node_t *em_node_new(em_node_type_t type, em_pos_t *pos) {

	em_node_t *node = EM_NODE(em_refobj_new(&em_reflist_node, sizeof(em_node_t), EM_CLEANUP_MODE_IMMEDIATE));
	if (!node) return NULL;

	EM_REFOBJ(node)->free = node_free;

	node->type = type;
	memcpy(&node->pos, pos, sizeof(node->pos));
	node->flags = 0;
	node->parent = NULL;
	node->first = NULL;
	node->last = NULL;
	node->prev = NULL;
	node->next = NULL;
	node->tokens = EM_ARRAY_INIT;

	if (em_array_init(&node->tokens) != EM_RESULT_SUCCESS) {

		em_refobj_decref(EM_REFOBJ(node));
		return NULL;
	}
	return node;
}

/* add child node */
EM_API void em_node_add_child(em_node_t *node, em_node_t *child) {

	if (!node || !child) return;

	child->prev = node->last;
	if (!node->first) node->first = child;
	if (node->last) node->last->next = child;
	node->last = child;

	child->parent = node;

	EM_NODE_INCREF(child);
}

/* add token */
EM_API void em_node_add_token(em_node_t *node, em_token_t *token) {

	if (!node || !token) return;

	em_generic_t value = {.t_voidp = token};
	(void)em_array_add(&node->tokens, value);

	EM_TOKEN_INCREF(token);
}

/* get token */
EM_API em_token_t *em_node_get_token(em_node_t *node, size_t index) {

	if (!node) return NULL;

	em_generic_result_t res = em_array_get(&node->tokens, index);
	if (!res.p) return NULL;

	return (em_token_t *)res.v.t_voidp;
}

/* print node information */
static void print_node(em_node_t *node, int level) {

	for (int i = 0; i < level; i++) printf("  ");
	printf("<%s:%u", em_get_node_type_name(node->type), node->flags);

	/* include tokens */
	if (node->tokens.nitems) {

		printf(" (");
		for (size_t i = 0; i < node->tokens.nitems; i++) {

			if (i) printf(", ");

			em_token_t *token = em_node_get_token(node, i);
			printf("%s:'%s'", em_get_token_type_name(token->type), token->value);
		}
		printf(")");
	}
	printf(">\n");

	/* include children */
	em_node_t *cur = node->first;
	while (cur) {

		print_node(cur, level+2);
		cur = cur->next;
	}
}

EM_API void em_node_print(em_node_t *node) {

	print_node(node, 0);
}
