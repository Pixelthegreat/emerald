/*
 * Copyright 2026, Elliot Kohlmyer
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

/* functions */
EM_API em_value_t em_byte_array_new(size_t size, em_byte_array_mode_t mode); /* create byte array */
EM_API void em_byte_array_set(em_value_t object, em_ssize_t index, em_inttype_t value); /* set value in byte array */
EM_API em_inttype_t em_byte_array_get(em_value_t object, em_ssize_t index); /* get value from byte array */
EM_API em_bool_t em_is_byte_array(em_value_t v); /* determine if value is byte array */

#endif /* EMERALD_MODULE_ARRAY_H */
