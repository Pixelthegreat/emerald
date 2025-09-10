/*
 * Copyright 2025, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <emerald/core.h>
#include <emerald/memory.h>
#include <emerald/string.h>
#include <emerald/list.h>

/* type */
static em_value_t is_true(em_value_t v, em_pos_t *pos);
static em_value_t get_by_index(em_value_t v, em_value_t i, em_pos_t *pos);
static em_result_t set_by_index(em_value_t a, em_value_t i, em_value_t b, em_pos_t *pos);
static em_value_t length_of(em_value_t v, em_pos_t *pos);
static em_value_t to_string(em_value_t v, em_pos_t *pos);

static em_object_type_t type = {
	.is_true = is_true,
	.get_by_index = get_by_index,
	.set_by_index = set_by_index,
	.length_of = length_of,
	.to_string = to_string,
};

/* is value true */
static em_value_t is_true(em_value_t v, em_pos_t *pos) {

	em_list_t *list = EM_LIST(EM_OBJECT_FROM_VALUE(v));
	return list->nitems? EM_VALUE_TRUE: EM_VALUE_FALSE;
}

/* get value by index */
static em_value_t get_by_index(em_value_t v, em_value_t i, em_pos_t *pos) {

	em_list_t *list = EM_LIST(EM_OBJECT_FROM_VALUE(v));
	if (i.type != EM_VALUE_TYPE_INT)
		return EM_VALUE_FAIL;

	return em_list_get(v, (em_ssize_t)i.value.te_inttype);
}

/* set value by index */
static em_result_t set_by_index(em_value_t a, em_value_t i, em_value_t b, em_pos_t *pos) {

	em_list_t *list = EM_LIST(EM_OBJECT_FROM_VALUE(a));

	if (i.type != EM_VALUE_TYPE_INT ||
	    i.value.te_inttype < -(em_inttype_t)list->nitems ||
	    i.value.te_inttype >= (em_inttype_t)list->nitems)
		return EM_RESULT_FAILURE;

	em_list_set(a, (em_ssize_t)i.value.te_inttype, b);
	return EM_RESULT_SUCCESS;
}

/* get length of list */
static em_value_t length_of(em_value_t v, em_pos_t *pos) {

	em_list_t *list = EM_LIST(EM_OBJECT_FROM_VALUE(v));
	return EM_VALUE_INT((em_inttype_t)list->nitems);
}

/* get string representation */
static em_value_t to_string(em_value_t v, em_pos_t *pos) {

	return em_string_new_from_utf8("[...]", 5);
}

/* free list */
static void list_free(void *p) {

	em_list_t *list = EM_LIST(p);

	for (size_t i = 0; i < list->nitems; i++) {
		if (i < list->nbase) em_value_decref(list->items[i]);
		else em_value_decref(list->ext[i-list->nbase]);
	}
	if (list->ext) em_free(list->ext);
}

/* create list */
EM_API em_value_t em_list_new(size_t nbase) {

	em_value_t value = em_object_new(&type, sizeof(em_list_t) + sizeof(em_value_t) * nbase);
	em_list_t *list = EM_LIST(EM_OBJECT_FROM_VALUE(value));

	EM_REFOBJ(list)->free = list_free;

	list->nitems = 0;
	list->nbase = 0;
	list->ext = NULL;
	list->next = 0;
	list->cext = 0;

	return value;
}

/* set value in list */
EM_API void em_list_set(em_value_t object, em_ssize_t index, em_value_t value) {

	em_list_t *list = EM_LIST(EM_OBJECT_FROM_VALUE(object));
	if (index < 0) index = (em_ssize_t)list->nitems + index;

	if (index < 0 || index >= (em_ssize_t)list->nitems)
		return;

	em_value_t *slot = NULL;
	if (index < list->nbase) slot = &list->items[index];
	else slot = &list->ext[index - (em_ssize_t)list->nbase];

	em_value_decref(*slot);
	*slot = value;
	em_value_incref(value);
}

/* get value from list */
EM_API em_value_t em_list_get(em_value_t object, em_ssize_t index) {

	em_list_t *list = EM_LIST(EM_OBJECT_FROM_VALUE(object));
	if (index < 0) index = (em_ssize_t)list->nitems + index;

	if (index < 0 || index >= (em_ssize_t)list->nitems)
		return EM_VALUE_FAIL;

	if (index < list->nbase) return list->items[index];
	else return list->ext[index - (em_ssize_t)list->nbase];
}

/* add item to list */
EM_API void em_list_append(em_value_t object, em_value_t value) {

	em_list_t *list = EM_LIST(EM_OBJECT_FROM_VALUE(object));

	/* base item list */
	if (list->nitems < list->nbase)
		list->items[list->nitems++] = value;

	/* extension list */
	else {

		if (!list->ext) {

			list->next = 0;
			list->cext = 16;
			list->ext = em_malloc(sizeof(em_value_t) * list->cext);
		}
		else if (list->next >= list->cext) {

			list->cext *= 2;
			list->ext = em_realloc(list->ext, sizeof(em_value_t) * list->cext);
		}
		list->ext[list->next++] = value;
		list->nitems++;
	}

	em_value_incref(value);
}

/* determine if value is list */
EM_API em_bool_t em_is_list(em_value_t v) {

	return v.type == EM_VALUE_TYPE_OBJECT &&
	       EM_OBJECT_FROM_VALUE(v)->type == &type;
}
