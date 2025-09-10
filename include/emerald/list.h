/*
 * Copyright 2025, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef EMERALD_LIST_H
#define EMERALD_LIST_H

#include <emerald/core.h>
#include <emerald/value.h>
#include <emerald/object.h>

/* list */
typedef struct em_list {
	em_object_t base;
	size_t nitems; /* number of items */
	size_t nbase; /* base item list capacity */
	em_value_t *ext; /* extended list */
	size_t next, cext; /* number and capacity of extended list */
	em_value_t items[]; /* base items */
} em_list_t;

#define EM_LIST(p) ((em_list_t *)(p))

/* functions */
EM_API em_value_t em_list_new(size_t nbase); /* create list */
EM_API void em_list_set(em_value_t object, em_ssize_t index, em_value_t value); /* set value in list */
EM_API em_value_t em_list_get(em_value_t object, em_ssize_t index); /* get value from list */
EM_API void em_list_append(em_value_t object, em_value_t value); /* add item to list */
EM_API em_bool_t em_is_list(em_value_t v); /* determine if value is list */

#endif /* EMERALD_LIST_H */
