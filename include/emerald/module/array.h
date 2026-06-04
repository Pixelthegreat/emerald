/*
 * Copyright 2025-2026, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef EMERALD_MODULE_ARRAY_H
#define EMERALD_MODULE_ARRAY_H

#include <emerald/core.h>
#include <emerald/value.h>
#include <emerald/object.h>
#include <emerald/module.h>

EM_API em_module_t em_module_array;

/* byte array modes */
typedef enum em_byte_array_mode {
	EM_BYTE_ARRAY_MODE_CHAR = 0,
	EM_BYTE_ARRAY_MODE_UNSIGNED_CHAR,
	EM_BYTE_ARRAY_MODE_SHORT,
	EM_BYTE_ARRAY_MODE_UNSIGNED_SHORT,
	EM_BYTE_ARRAY_MODE_INT,
	EM_BYTE_ARRAY_MODE_UNSIGNED_INT,
	EM_BYTE_ARRAY_MODE_LONG,

	EM_BYTE_ARRAY_MODE_COUNT,
} em_byte_array_mode_t;

/* byte array */
typedef struct em_byte_array {
	em_object_t base;
	size_t size; /* size of array */
	em_byte_array_mode_t mode; /* array mode */
	uint8_t data[]; /* array data */
} em_byte_array_t;

#define EM_BYTE_ARRAY(p) ((em_byte_array_t *)(p))

/* byte array view */
typedef struct em_byte_array_view {
	em_object_t base;
	em_value_t array; /* attached array */
	size_t start; /* starting index */
	size_t count; /* number of items to view */
} em_byte_array_view_t;

#define EM_BYTE_ARRAY_VIEW(p) ((em_byte_array_view_t *)(p))

/* functions */
EM_API em_value_t em_byte_array_new(size_t size, em_byte_array_mode_t mode); /* create byte array */
EM_API void em_byte_array_set(em_value_t object, em_ssize_t index, em_inttype_t value); /* set value in byte array */
EM_API em_inttype_t em_byte_array_get(em_value_t object, em_ssize_t index); /* get value from byte array */
EM_API void em_byte_array_slice(em_value_t object, em_value_t other, em_ssize_t index); /* extract range of values from array */
EM_API em_bool_t em_is_byte_array(em_value_t v); /* determine if value is byte array */

EM_API em_value_t em_byte_array_view_new(void); /* create byte array view */
EM_API void em_byte_array_view_set_array(em_value_t object, em_value_t array, size_t start, size_t count); /* attach array to view */
EM_API void em_byte_array_view_set(em_value_t object, em_ssize_t index, em_inttype_t value); /* set value in view */
EM_API em_inttype_t em_byte_array_view_get(em_value_t object, em_ssize_t index); /* get value from view */
EM_API em_bool_t em_is_byte_array_view(em_value_t v); /* determine if value is byte array view */

#endif /* EMERALD_MODULE_ARRAY_H */
