/*
 * Copyright 2025, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef EMERALD_FUNCTION_H
#define EMERALD_FUNCTION_H

#include <emerald/core.h>
#include <emerald/node.h>
#include <emerald/object.h>

#define EM_FUNCTION_MAX_ARGUMENTS 32 /* arbitrary maximum */

/* builtin function */
typedef em_value_t (*em_builtin_function_handler_t)(struct em_context *, em_value_t *, size_t, em_pos_t *);

typedef struct em_builtin_function {
	em_object_t base;
	const char *name; /* function name */
	em_builtin_function_handler_t handler; /* c function to call */
} em_builtin_function_t;

#define EM_BUILTIN_FUNCTION(p) ((em_builtin_function_t *)(p))

/* function */
typedef struct em_function {
	em_object_t base;
	em_node_t *function_node; /* main function node */
	em_node_t *body_node; /* function body node */
	const char *name; /* function name */
	size_t nargnames; /* number of argument names */
	const char *argnames[]; /* argument names */
} em_function_t;

#define EM_FUNCTION(p) ((em_function_t *)(p))

/* functions */
EM_API em_value_t em_builtin_function_new(const char *name, em_builtin_function_handler_t handler); /* create builtin function */
EM_API em_value_t em_function_new(em_node_t *function_node, em_node_t *body_node, const char *name, size_t nargnames, const char **argnames); /* create function */

EM_API em_bool_t em_is_builtin_function(em_value_t v); /* check if value is builtin function */
EM_API em_bool_t em_is_function(em_value_t v); /* check if value is function */

#endif /* EMERALD_FUNCTION_H */
