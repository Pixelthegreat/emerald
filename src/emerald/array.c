/*
 * Copyright 2026, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <emerald/core.h>
#include <emerald/memory.h>
#include <emerald/array.h>

/* initialize array */
EM_API em_result_t em_array_init(em_array_t *array) {

	if (!array || array->init) return EM_RESULT_FAILURE;

	array->ext = NULL;
	array->nitems = 0;
	array->cext = 0;

	array->init = EM_TRUE;
	return EM_RESULT_SUCCESS;
}

/* add value */
EM_API em_result_t em_array_add(em_array_t *array, em_generic_t value) {

	if (!array || !array->init) return EM_RESULT_FAILURE;

	/* base array value */
	if (array->nitems < EM_ARRAY_BASE_ITEMS)
		array->base[array->nitems++] = value;

	/* extended array value */
	else {

		size_t index = array->nitems - EM_ARRAY_BASE_ITEMS;
		if (!array->ext) {

			array->cext = EM_ARRAY_BASE_ITEMS;
			array->ext = (em_generic_t *)em_malloc(sizeof(em_generic_t) * array->cext);
		}

		/* resize array */
		else if (index >= array->cext) {

			array->cext *= 2;
			array->ext = (em_generic_t *)em_realloc(array->ext, sizeof(em_generic_t) * array->cext);
		}
		array->ext[index] = value;
		array->nitems++;
	}
	return EM_RESULT_SUCCESS;
}

/* get array value */
EM_API em_generic_result_t em_array_get(em_array_t *array, size_t index) {

	if (!array || !array->init || index >= array->nitems) return EM_GENERIC_NONE;

	em_generic_result_t res = {.p = EM_TRUE};

	if (index < EM_ARRAY_BASE_ITEMS)
		res.v = array->base[index];
	else res.v = array->ext[index - EM_ARRAY_BASE_ITEMS];
	return res;
}

/* destroy array */
EM_API void em_array_destroy(em_array_t *array) {

	if (!array || !array->init) return;

	if (array->ext) free(array->ext);
	array->init = EM_FALSE;
}
