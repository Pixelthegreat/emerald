/*
 * Copyright 2025, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef EMERALD_VALUE_H
#define EMERALD_VALUE_H

#include <emerald/core.h>
#include <emerald/log.h>
#include <emerald/array.h>

/* value types */
typedef enum em_value_type {
	EM_VALUE_TYPE_NONE = 0,

	EM_VALUE_TYPE_INT,
	EM_VALUE_TYPE_FLOAT,
	EM_VALUE_TYPE_OBJECT,

	EM_VALUE_TYPE_COUNT,
} em_value_type_t;

/* value */
typedef struct em_value {
	em_value_type_t type; /* type of value */
	em_generic_t value; /* value of value */
} em_value_t;

#define EM_VALUE_INT(v) ((em_value_t){.type = EM_VALUE_TYPE_INT, .value.te_inttype = (v)})
#define EM_VALUE_FLOAT(v) ((em_value_t){.type = EM_VALUE_TYPE_FLOAT, .value.te_floattype = (v)})
#define EM_VALUE_INT_INV(v) ((em_value_t){.type = EM_VALUE_TYPE_INT, .value.te_inttype = !(v).value.te_inttype})

#define EM_VALUE_OK(v) ((v).type != EM_VALUE_TYPE_NONE)

#define EM_VALUE_FAIL ((em_value_t){.type = EM_VALUE_TYPE_NONE})

#define EM_VALUE_TRUE EM_VALUE_INT(1)
#define EM_VALUE_FALSE EM_VALUE_INT(0)

/* functions */
EM_API void em_value_incref(em_value_t v); /* increase reference count */
EM_API void em_value_decref(em_value_t v); /* decrease reference count */
EM_API void em_value_delete(em_value_t v); /* delete if reference count is zero */
EM_API em_value_t em_value_is_true(em_value_t v, em_pos_t *pos); /* get truthiness of value */
EM_API em_value_t em_value_add(em_value_t a, em_value_t b, em_pos_t *pos); /* add values */
EM_API em_value_t em_value_subtract(em_value_t a, em_value_t b, em_pos_t *pos); /* subtract values */
EM_API em_value_t em_value_multiply(em_value_t a, em_value_t b, em_pos_t *pos); /* multiply values */
EM_API em_value_t em_value_divide(em_value_t a, em_value_t b, em_pos_t *pos); /* divide values */
EM_API em_value_t em_value_modulo(em_value_t a, em_value_t b, em_pos_t *pos); /* modulo values */
EM_API em_value_t em_value_or(em_value_t a, em_value_t b, em_pos_t *pos); /* or values */
EM_API em_value_t em_value_and(em_value_t a, em_value_t b, em_pos_t *pos); /* and values */
EM_API em_value_t em_value_shift_left(em_value_t a, em_value_t b, em_pos_t *pos); /* shift left */
EM_API em_value_t em_value_shift_right(em_value_t a, em_value_t b, em_pos_t *pos); /* shift right */
EM_API em_value_t em_value_compare_equal(em_value_t a, em_value_t b, em_pos_t *pos); /* compare if values are equal */
EM_API em_value_t em_value_compare_less_than(em_value_t a, em_value_t b, em_pos_t *pos); /* compare if value is less than other */
EM_API em_value_t em_value_compare_greater_than(em_value_t a, em_value_t b, em_pos_t *pos); /* compare if value is greater than other */
EM_API em_value_t em_value_compare_or(em_value_t a, em_value_t b, em_pos_t *pos); /* or truthiness of values */
EM_API em_value_t em_value_compare_and(em_value_t a, em_value_t b, em_pos_t *pos); /* and truthiness of values */
EM_API em_hash_t em_value_hash(em_value_t v, em_pos_t *pos); /* get hash value */
EM_API em_value_t em_value_get_by_hash(em_value_t v, em_hash_t hash, em_pos_t *pos); /* get value by key hash */
EM_API em_value_t em_value_get_by_index(em_value_t v, em_value_t i, em_pos_t *pos); /* get value by index value */
EM_API em_result_t em_value_set_by_hash(em_value_t a, em_hash_t hash, em_value_t b, em_pos_t *pos); /* set value by key hash */
EM_API em_result_t em_value_set_by_index(em_value_t a, em_value_t i, em_value_t b, em_pos_t *pos); /* set value by index */
EM_API em_value_t em_value_call(struct em_context *context, em_value_t v, em_value_t *args, size_t nargs, em_pos_t *pos); /* call value */
EM_API em_value_t em_value_length_of(em_value_t v, em_pos_t *pos); /* get value length */
EM_API em_value_t em_value_to_string(em_value_t v, em_pos_t *pos); /* get string representation of value */
EM_API void em_value_log(em_value_t v); /* log value */

#endif /* EMERALD_VALUE_H */
