/*
 * Copyright 2026, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef EMERALD_CLASS_H
#define EMERALD_CLASS_H

#include <emerald/core.h>
#include <emerald/node.h>
#include <emerald/object.h>
#include <emerald/map.h>

/* bound method */
typedef struct em_method {
	em_object_t base;
	em_value_t binding; /* binding value */
	em_value_t function; /* function to call */
} em_method_t;

#define EM_METHOD(p) ((em_method_t *)(p))

/* class */
typedef struct em_class {
	em_object_t base;
	em_node_t *node; /* class node */
	const char *name; /* class name */
	em_value_t clsbase; /* base class */
	em_value_t map; /* value map */
} em_class_t;

#define EM_CLASS(p) ((em_class_t *)(p))

/* functions */
EM_API em_value_t em_method_new(em_value_t binding, em_value_t function); /* create bound method */
EM_API em_value_t em_class_new(em_node_t *node, const char *name, em_value_t base, em_value_t map); /* create class */

EM_API em_bool_t em_is_method(em_value_t v); /* check if value is method */
EM_API em_bool_t em_is_class(em_value_t v); /* check if value is class */
EM_API em_bool_t em_class_inherits(em_value_t cls, em_value_t base); /* check if a class inherits a base class */

#endif /* EMERALD_CLASS_H */
