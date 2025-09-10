/*
 * Copyright 2025, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <emerald/core.h>
#include <emerald/wchar.h>
#include <emerald/hash.h>
#include <emerald/string.h>

#define INVALID_OPERATION ({\
		em_log_runtime_error(pos, "Invalid operation");\
		return EM_VALUE_FAIL;\
	})

/* object type */
static em_value_t is_true(em_value_t v, em_pos_t *pos);
static em_value_t add(em_value_t a, em_value_t b, em_pos_t *pos);
static em_value_t multiply(em_value_t a, em_value_t b, em_pos_t *pos);
static em_value_t compare_equal(em_value_t a, em_value_t b, em_pos_t *pos);
static em_hash_t hash(em_value_t v, em_pos_t *pos);
static em_value_t to_string(em_value_t v, em_pos_t *pos);

static em_object_type_t type = {
	.is_true = is_true,
	.add = add,
	.multiply = multiply,
	.compare_equal = compare_equal,
	.hash = hash,
	.to_string = to_string,
};

/* get truthiness */
static em_value_t is_true(em_value_t v, em_pos_t *pos) {

	em_string_t *string = EM_STRING(EM_OBJECT_FROM_VALUE(v));

	return string->length? EM_VALUE_TRUE: EM_VALUE_FALSE;
}

/* add strings */
static em_value_t add(em_value_t a, em_value_t b, em_pos_t *pos) {

	if (b.type != EM_VALUE_TYPE_OBJECT ||
	    EM_OBJECT_FROM_VALUE(b)->type != &type)
		INVALID_OPERATION;

	em_string_t *first = EM_STRING(EM_OBJECT_FROM_VALUE(a));
	em_string_t *second = EM_STRING(EM_OBJECT_FROM_VALUE(b));

	em_value_t result = em_string_new(first->length + second->length);
	em_string_t *string = EM_STRING(EM_OBJECT_FROM_VALUE(result));

	memcpy(string->data, first->data, first->length * sizeof(em_wchar_t));
	memcpy(string->data + first->length, second->data, second->length * sizeof(em_wchar_t));
	string->data[string->length] = EM_INT2WC(0);

	string->hash = em_wchar_strhash(string->data);
	return result;
}

/* repeat string */
static em_value_t multiply(em_value_t a, em_value_t b, em_pos_t *pos) {

	if (b.type != EM_VALUE_TYPE_INT)
		INVALID_OPERATION;

	em_string_t *string = EM_STRING(EM_OBJECT_FROM_VALUE((a)));
	em_inttype_t repeat_count = b.value.te_inttype;

	if (repeat_count < 0 || repeat_count >= 1024) {

		em_log_runtime_error(pos, "Invalid repeat count of '%ld'", (long)repeat_count);
		return EM_VALUE_FAIL;
	}

	em_value_t result = em_string_new(string->length * (size_t)repeat_count);
	em_string_t *new = EM_STRING(EM_OBJECT_FROM_VALUE(result));

	for (em_inttype_t i = 0; i < repeat_count; i++)
		memcpy(new->data + (size_t)i * string->length, string->data, string->length * sizeof(em_wchar_t));
	new->data[new->length] = EM_INT2WC(0);

	new->hash = em_wchar_strhash(new->data);
	return result;
}

/* compare equality */
static em_value_t compare_equal(em_value_t a, em_value_t b, em_pos_t *pos) {

	em_string_t *first = EM_STRING(EM_OBJECT_FROM_VALUE(a));
	em_string_t *second = EM_STRING(EM_OBJECT_FROM_VALUE(b));

	if (first->length != second->length ||
	    first->hash != second->hash)
		return EM_VALUE_FALSE;

	return !memcmp(first->data, second->data, first->length)? EM_VALUE_TRUE: EM_VALUE_FALSE;
}

/* get hash value */
static em_hash_t hash(em_value_t v, em_pos_t *pos) {

	em_string_t *string = EM_STRING(EM_OBJECT_FROM_VALUE(v));
	return string->hash;
}

/* get string representation */
static em_value_t to_string(em_value_t v, em_pos_t *pos) {

	return v;
}

/* create string */
EM_API em_value_t em_string_new(size_t length) {

	em_value_t value = em_object_new(&type, sizeof(em_string_t) + (length + 1) * sizeof(em_wchar_t));
	em_string_t *string = EM_STRING(EM_OBJECT_FROM_VALUE(value));

	string->length = length;
	string->hash = 0;
	string->data[0] = EM_INT2WC(0);

	return value;
}

/* create string from utf8 data */
EM_API em_value_t em_string_new_from_utf8(const char *data, size_t length) {

	em_value_t value = em_string_new(length);
	em_string_t *string = EM_STRING(EM_OBJECT_FROM_VALUE(value));

	em_wchar_from_utf8(string->data, length+1, data);
	string->data[length] = EM_INT2WC(0);

	string->hash = em_wchar_strhash(string->data);
	return value;
}

/* create string from wchar data */
EM_API em_value_t em_string_new_from_wchar(const em_wchar_t *data, size_t length) {

	em_value_t value = em_string_new(length);
	em_string_t *string = EM_STRING(EM_OBJECT_FROM_VALUE(value));

	memcpy(string->data, data, length * sizeof(em_wchar_t));
	string->data[length] = EM_INT2WC(0);

	string->hash = em_wchar_strhash(string->data);
	return value;
}

/* determine if value is a string */
EM_API em_bool_t em_is_string(em_value_t v) {

	return v.type == EM_VALUE_TYPE_OBJECT &&
	       EM_OBJECT_FROM_VALUE(v)->type == &type;
}
