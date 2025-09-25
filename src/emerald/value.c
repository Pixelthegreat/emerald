/*
 * Copyright 2025, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <emerald/core.h>
#include <emerald/utf8.h>
#include <emerald/wchar.h>
#include <emerald/object.h>
#include <emerald/string.h>
#include <emerald/value.h>

#define INVALID_OPERATION_RETURN(retv) ({\
		em_log_runtime_error(pos, "Invalid operation");\
		return retv;\
	})
#define INVALID_OPERATION INVALID_OPERATION_RETURN(EM_VALUE_FAIL)

/* operations */
static em_value_t is_true_int(em_value_t v, em_pos_t *pos);
static em_value_t add_int(em_value_t a, em_value_t b, em_pos_t *pos);
static em_value_t subtract_int(em_value_t a, em_value_t b, em_pos_t *pos);
static em_value_t multiply_int(em_value_t a, em_value_t b, em_pos_t *pos);
static em_value_t divide_int(em_value_t a, em_value_t b, em_pos_t *pos);
static em_value_t modulo_int(em_value_t a, em_value_t b, em_pos_t *pos);
static em_value_t or_int(em_value_t a, em_value_t b, em_pos_t *pos);
static em_value_t and_int(em_value_t a, em_value_t b, em_pos_t *pos);
static em_value_t shift_left_int(em_value_t a, em_value_t b, em_pos_t *pos);
static em_value_t shift_right_int(em_value_t a, em_value_t b, em_pos_t *pos);
static em_value_t compare_equal_int(em_value_t a, em_value_t b, em_pos_t *pos);
static em_value_t compare_less_than_int(em_value_t a, em_value_t b, em_pos_t *pos);
static em_value_t compare_greater_than_int(em_value_t a, em_value_t b, em_pos_t *pos);
static em_hash_t hash_int(em_value_t v, em_pos_t *pos);
static em_value_t to_string_int(em_value_t v, em_pos_t *pos);

static em_value_t is_true_float(em_value_t v, em_pos_t *pos);
static em_value_t add_float(em_value_t a, em_value_t b, em_pos_t *pos);
static em_value_t subtract_float(em_value_t a, em_value_t b, em_pos_t *pos);
static em_value_t multiply_float(em_value_t a, em_value_t b, em_pos_t *pos);
static em_value_t divide_float(em_value_t a, em_value_t b, em_pos_t *pos);
static em_value_t modulo_float(em_value_t a, em_value_t b, em_pos_t *pos);
static em_value_t compare_equal_float(em_value_t a, em_value_t b, em_pos_t *pos);
static em_value_t compare_less_than_float(em_value_t a, em_value_t b, em_pos_t *pos);
static em_value_t compare_greater_than_float(em_value_t a, em_value_t b, em_pos_t *pos);
static em_hash_t hash_float(em_value_t v, em_pos_t *pos);
static em_value_t to_string_float(em_value_t v, em_pos_t *pos);

struct {
	em_value_t (*is_true)(em_value_t, em_pos_t *);
	em_value_t (*add)(em_value_t, em_value_t, em_pos_t *);
	em_value_t (*subtract)(em_value_t, em_value_t, em_pos_t *);
	em_value_t (*multiply)(em_value_t, em_value_t, em_pos_t *);
	em_value_t (*divide)(em_value_t, em_value_t, em_pos_t *);
	em_value_t (*modulo)(em_value_t, em_value_t, em_pos_t *);
	em_value_t (*or)(em_value_t, em_value_t, em_pos_t *);
	em_value_t (*and)(em_value_t, em_value_t, em_pos_t *);
	em_value_t (*shift_left)(em_value_t, em_value_t, em_pos_t *);
	em_value_t (*shift_right)(em_value_t, em_value_t, em_pos_t *);
	em_value_t (*compare_equal)(em_value_t, em_value_t, em_pos_t *);
	em_value_t (*compare_less_than)(em_value_t, em_value_t, em_pos_t *);
	em_value_t (*compare_greater_than)(em_value_t, em_value_t, em_pos_t *);
	em_hash_t (*hash)(em_value_t, em_pos_t *);
	em_value_t (*get_by_hash)(em_value_t, em_hash_t, em_pos_t *);
	em_value_t (*get_by_index)(em_value_t, em_value_t, em_pos_t *);
	em_result_t (*set_by_hash)(em_value_t, em_hash_t, em_value_t, em_pos_t *);
	em_result_t (*set_by_index)(em_value_t, em_value_t, em_value_t, em_pos_t *);
	em_value_t (*call)(struct em_context *, em_value_t, em_value_t *, size_t, em_pos_t *);
	em_value_t (*length_of)(em_value_t, em_pos_t *);
	em_value_t (*to_string)(em_value_t, em_pos_t *);

} ops[EM_VALUE_TYPE_COUNT] = {
	[EM_VALUE_TYPE_INT] = {
		.is_true = is_true_int,
		.add = add_int,
		.subtract = subtract_int,
		.multiply = multiply_int,
		.divide = divide_int,
		.modulo = modulo_int,
		.or = or_int,
		.and = and_int,
		.shift_left = shift_left_int,
		.shift_right = shift_right_int,
		.compare_equal = compare_equal_int,
		.compare_less_than = compare_less_than_int,
		.compare_greater_than = compare_greater_than_int,
		.hash = hash_int,
		.to_string = to_string_int,
	},
	[EM_VALUE_TYPE_FLOAT] = {
		.is_true = is_true_float,
		.add = add_float,
		.subtract = subtract_float,
		.multiply = multiply_float,
		.divide = divide_float,
		.modulo = modulo_float,
		.compare_equal = compare_equal_float,
		.compare_less_than = compare_less_than_float,
		.compare_greater_than = compare_greater_than_float,
		.hash = hash_float,
		.to_string = to_string_float,
	},
	[EM_VALUE_TYPE_OBJECT] = {
		.is_true = em_object_is_true,
		.add = em_object_add,
		.subtract = em_object_subtract,
		.multiply = em_object_multiply,
		.divide = em_object_divide,
		.modulo = em_object_modulo,
		.or = em_object_or,
		.and = em_object_and,
		.shift_left = em_object_shift_left,
		.shift_right = em_object_shift_right,
		.compare_equal = em_object_compare_equal,
		.compare_less_than = em_object_compare_less_than,
		.compare_greater_than = em_object_compare_greater_than,
		.hash = em_object_hash,
		.get_by_hash = em_object_get_by_hash,
		.get_by_index = em_object_get_by_index,
		.set_by_hash = em_object_set_by_hash,
		.set_by_index = em_object_set_by_index,
		.call = em_object_call,
		.length_of = em_object_length_of,
		.to_string = em_object_to_string,
	},
};

/* is int true */
static em_value_t is_true_int(em_value_t v, em_pos_t *pos) {

	return EM_VALUE_INT(v.value.te_inttype != 0);
}

/* add ints */
static em_value_t add_int(em_value_t a, em_value_t b, em_pos_t *pos) {

	switch (b.type) {
		case EM_VALUE_TYPE_INT:
			return EM_VALUE_INT(a.value.te_inttype + b.value.te_inttype);
		case EM_VALUE_TYPE_FLOAT:
			return EM_VALUE_FLOAT((em_floattype_t)a.value.te_inttype + b.value.te_floattype);
		default:
			INVALID_OPERATION;
	}
}

/* subtract ints */
static em_value_t subtract_int(em_value_t a, em_value_t b, em_pos_t *pos) {

	switch (b.type) {
		case EM_VALUE_TYPE_INT:
			return EM_VALUE_INT(a.value.te_inttype - b.value.te_inttype);
		case EM_VALUE_TYPE_FLOAT:
			return EM_VALUE_FLOAT((em_floattype_t)a.value.te_inttype - b.value.te_floattype);
		default:
			INVALID_OPERATION;
	}
}

/* multiply ints */
static em_value_t multiply_int(em_value_t a, em_value_t b, em_pos_t *pos) {

	switch (b.type) {
		case EM_VALUE_TYPE_INT:
			return EM_VALUE_INT(a.value.te_inttype * b.value.te_inttype);
		case EM_VALUE_TYPE_FLOAT:
			return EM_VALUE_FLOAT((em_floattype_t)a.value.te_inttype * b.value.te_floattype);
		default:
			INVALID_OPERATION;
	}
}

/* divide ints */
static em_value_t divide_int(em_value_t a, em_value_t b, em_pos_t *pos) {

	switch (b.type) {
		case EM_VALUE_TYPE_INT:
			return EM_VALUE_INT(a.value.te_inttype / b.value.te_inttype);
		case EM_VALUE_TYPE_FLOAT:
			return EM_VALUE_FLOAT((em_floattype_t)a.value.te_inttype / b.value.te_floattype);
		default:
			INVALID_OPERATION;
	}
}

/* modulo ints */
static em_value_t modulo_int(em_value_t a, em_value_t b, em_pos_t *pos) {

	switch (b.type) {
		case EM_VALUE_TYPE_INT:
			return EM_VALUE_INT(a.value.te_inttype % b.value.te_inttype);
		case EM_VALUE_TYPE_FLOAT:
			return EM_VALUE_FLOAT(EM_FLOATTYPE_MOD((em_floattype_t)a.value.te_inttype, b.value.te_floattype));
		default:
			INVALID_OPERATION;
	}
}

/* or ints */
static em_value_t or_int(em_value_t a, em_value_t b, em_pos_t *pos) {

	if (b.type != EM_VALUE_TYPE_INT) INVALID_OPERATION;

	return EM_VALUE_INT(a.value.te_inttype | b.value.te_inttype);
}

/* and ints */
static em_value_t and_int(em_value_t a, em_value_t b, em_pos_t *pos) {

	if (b.type != EM_VALUE_TYPE_INT) INVALID_OPERATION;

	return EM_VALUE_INT(a.value.te_inttype & b.value.te_inttype);
}

/* shift left ints */
static em_value_t shift_left_int(em_value_t a, em_value_t b, em_pos_t *pos) {

	if (b.type != EM_VALUE_TYPE_INT) INVALID_OPERATION;

	return EM_VALUE_INT(a.value.te_inttype << b.value.te_inttype);
}

/* shift right ints */
static em_value_t shift_right_int(em_value_t a, em_value_t b, em_pos_t *pos) {

	if (b.type != EM_VALUE_TYPE_INT) INVALID_OPERATION;

	return EM_VALUE_INT(a.value.te_inttype >> b.value.te_inttype);
}

/* compare if ints are equal */
static em_value_t compare_equal_int(em_value_t a, em_value_t b, em_pos_t *pos) {

	switch (b.type) {
		case EM_VALUE_TYPE_INT:
			return EM_VALUE_INT(a.value.te_inttype == b.value.te_inttype);
		case EM_VALUE_TYPE_FLOAT:
			return EM_VALUE_INT((em_floattype_t)a.value.te_inttype == b.value.te_floattype);
		default:
			return EM_VALUE_INT(0);
	}
}

/* compare if value is less than other */
static em_value_t compare_less_than_int(em_value_t a, em_value_t b, em_pos_t *pos) {

	switch (b.type) {
		case EM_VALUE_TYPE_INT:
			return EM_VALUE_INT(a.value.te_inttype < b.value.te_inttype);
		case EM_VALUE_TYPE_FLOAT:
			return EM_VALUE_INT((em_floattype_t)a.value.te_inttype < b.value.te_floattype);
		default:
			return EM_VALUE_INT(0);
	}
}

/* compare if value is greater than other */
static em_value_t compare_greater_than_int(em_value_t a, em_value_t b, em_pos_t *pos) {

	switch (b.type) {
		case EM_VALUE_TYPE_INT:
			return EM_VALUE_INT(a.value.te_inttype > b.value.te_inttype);
		case EM_VALUE_TYPE_FLOAT:
			return EM_VALUE_INT((em_floattype_t)a.value.te_inttype > b.value.te_floattype);
		default:
			return EM_VALUE_INT(0);
	}
}

/* hash integer */
static em_hash_t hash_int(em_value_t v, em_pos_t *pos) {

	return (em_hash_t)v.value.te_inttype;
}

/* get string representation of int */
static em_value_t to_string_int(em_value_t v, em_pos_t *pos) {

	char buf[64];
	snprintf(buf, 64, EM_INTTYPE_FORMAT, v.value.te_inttype);

	return em_string_new_from_utf8(buf, em_utf8_strlen(buf));
}

/* is float true */
static em_value_t is_true_float(em_value_t v, em_pos_t *pos) {

	return EM_VALUE_INT(v.value.te_floattype != 0.f);
}

/* add floats */
static em_value_t add_float(em_value_t a, em_value_t b, em_pos_t *pos) {

	switch (b.type) {
		case EM_VALUE_TYPE_INT:
			return EM_VALUE_FLOAT(a.value.te_floattype + (em_floattype_t)b.value.te_inttype);
		case EM_VALUE_TYPE_FLOAT:
			return EM_VALUE_FLOAT(a.value.te_floattype + b.value.te_floattype);
		default:
			INVALID_OPERATION;
	}
}

/* subtract floats */
static em_value_t subtract_float(em_value_t a, em_value_t b, em_pos_t *pos) {

	switch (b.type) {
		case EM_VALUE_TYPE_INT:
			return EM_VALUE_FLOAT(a.value.te_floattype - (em_floattype_t)b.value.te_inttype);
		case EM_VALUE_TYPE_FLOAT:
			return EM_VALUE_FLOAT(a.value.te_floattype - b.value.te_floattype);
		default:
			INVALID_OPERATION;
	}
}

/* multiply floats */
static em_value_t multiply_float(em_value_t a, em_value_t b, em_pos_t *pos) {

	switch (b.type) {
		case EM_VALUE_TYPE_INT:
			return EM_VALUE_FLOAT(a.value.te_floattype * (em_floattype_t)b.value.te_inttype);
		case EM_VALUE_TYPE_FLOAT:
			return EM_VALUE_FLOAT(a.value.te_floattype * b.value.te_floattype);
		default:
			INVALID_OPERATION;
	}
}

/* divide floats */
static em_value_t divide_float(em_value_t a, em_value_t b, em_pos_t *pos) {

	switch (b.type) {
		case EM_VALUE_TYPE_INT:
			return EM_VALUE_FLOAT(a.value.te_floattype / (em_floattype_t)b.value.te_inttype);
		case EM_VALUE_TYPE_FLOAT:
			return EM_VALUE_FLOAT(a.value.te_floattype / b.value.te_floattype);
		default:
			INVALID_OPERATION;
	}
}

/* modulo floats */
static em_value_t modulo_float(em_value_t a, em_value_t b, em_pos_t *pos) {

	switch (b.type) {
		case EM_VALUE_TYPE_INT:
			return EM_VALUE_FLOAT(EM_FLOATTYPE_MOD(a.value.te_floattype, (em_floattype_t)b.value.te_inttype));
		case EM_VALUE_TYPE_FLOAT:
			return EM_VALUE_FLOAT(EM_FLOATTYPE_MOD(a.value.te_floattype, b.value.te_floattype));
		default:
			INVALID_OPERATION;
	}
}

/* compare if floats are equal */
static em_value_t compare_equal_float(em_value_t a, em_value_t b, em_pos_t *pos) {

	switch (b.type) {
		case EM_VALUE_TYPE_INT:
			return EM_VALUE_INT(a.value.te_floattype == (em_floattype_t)b.value.te_inttype);
		case EM_VALUE_TYPE_FLOAT:
			return EM_VALUE_INT(a.value.te_floattype == b.value.te_floattype);
		default:
			return EM_VALUE_INT(0);
	}
}

/* compare if float is less than other */
static em_value_t compare_less_than_float(em_value_t a, em_value_t b, em_pos_t *pos) {

	switch (b.type) {
		case EM_VALUE_TYPE_INT:
			return EM_VALUE_INT(a.value.te_floattype < (em_floattype_t)b.value.te_inttype);
		case EM_VALUE_TYPE_FLOAT:
			return EM_VALUE_INT(a.value.te_floattype > b.value.te_floattype);
		default:
			return EM_VALUE_INT(0);
	}
}

/* compare if float is greater than other */
static em_value_t compare_greater_than_float(em_value_t a, em_value_t b, em_pos_t *pos) {

	switch (b.type) {
		case EM_VALUE_TYPE_INT:
			return EM_VALUE_INT(a.value.te_floattype > (em_floattype_t)b.value.te_inttype);
		case EM_VALUE_TYPE_FLOAT:
			return EM_VALUE_INT(a.value.te_floattype > b.value.te_floattype);
		default:
			return EM_VALUE_INT(0);
	}
}

/* hash float */
static em_hash_t hash_float(em_value_t v, em_pos_t *pos) {

	return *(em_hash_t *)&v.value.te_floattype;
}

/* get string representation of float */
static em_value_t to_string_float(em_value_t v, em_pos_t *pos) {

	char buf[64];
	snprintf(buf, 64, EM_FLOATTYPE_FORMAT, v.value.te_floattype);

	return em_string_new_from_utf8(buf, em_utf8_strlen(buf));
}

/* increase reference count */
EM_API void em_value_incref(em_value_t v) {

	if (v.type == EM_VALUE_TYPE_OBJECT)
		EM_OBJECT_INCREF(v.value.t_voidp);
}

/* decrease reference count */
EM_API void em_value_decref(em_value_t v) {

	if (v.type == EM_VALUE_TYPE_OBJECT)
		EM_OBJECT_DECREF(v.value.t_voidp);
}

/* delete if reference count is zero */
EM_API void em_value_delete(em_value_t v) {

	if (!EM_VALUE_OK(v)) return;

	if (v.type == EM_VALUE_TYPE_OBJECT) {

		EM_OBJECT_INCREF(v.value.t_voidp);
		EM_OBJECT_DECREF(v.value.t_voidp);
	}
}

/* get truthiness of value */
EM_API em_value_t em_value_is_true(em_value_t v, em_pos_t *pos) {

	return ops[v.type].is_true(v, pos);
}

/* add values */
EM_API em_value_t em_value_add(em_value_t a, em_value_t b, em_pos_t *pos) {

	return ops[a.type].add(a, b, pos);
}

/* subtract values */
EM_API em_value_t em_value_subtract(em_value_t a, em_value_t b, em_pos_t *pos) {

	return ops[a.type].subtract(a, b, pos);
}

/* multiply values */
EM_API em_value_t em_value_multiply(em_value_t a, em_value_t b, em_pos_t *pos) {

	return ops[a.type].multiply(a, b, pos);
}

/* divide values */
EM_API em_value_t em_value_divide(em_value_t a, em_value_t b, em_pos_t *pos) {

	return ops[a.type].divide(a, b, pos);
}

/* modulo values */
EM_API em_value_t em_value_modulo(em_value_t a, em_value_t b, em_pos_t *pos) {

	return ops[a.type].modulo(a, b, pos);
}

/* or values */
EM_API em_value_t em_value_or(em_value_t a, em_value_t b, em_pos_t *pos) {

	if (ops[a.type].or) return ops[a.type].or(a, b, pos);

	INVALID_OPERATION;
}

/* and values */
EM_API em_value_t em_value_and(em_value_t a, em_value_t b, em_pos_t *pos) {

	if (ops[a.type].and) return ops[a.type].and(a, b, pos);

	INVALID_OPERATION;
}

/* shift left */
EM_API em_value_t em_value_shift_left(em_value_t a, em_value_t b, em_pos_t *pos) {

	if (ops[a.type].shift_left) return ops[a.type].shift_left(a, b, pos);

	INVALID_OPERATION;
}

/* shift right */
EM_API em_value_t em_value_shift_right(em_value_t a, em_value_t b, em_pos_t *pos) {

	if (ops[a.type].shift_right) return ops[a.type].shift_right(a, b, pos);

	INVALID_OPERATION;
}

/* compare if values are equal */
EM_API em_value_t em_value_compare_equal(em_value_t a, em_value_t b, em_pos_t *pos) {

	if (ops[a.type].compare_equal) return ops[a.type].compare_equal(a, b, pos);

	return EM_VALUE_FALSE;
}

/* compare if value is less than other */
EM_API em_value_t em_value_compare_less_than(em_value_t a, em_value_t b, em_pos_t *pos) {

	if (ops[a.type].compare_less_than) return ops[a.type].compare_less_than(a, b, pos);

	INVALID_OPERATION;
}

/* compare if value is greater than other */
EM_API em_value_t em_value_compare_greater_than(em_value_t a, em_value_t b, em_pos_t *pos) {

	if (ops[a.type].compare_greater_than) return ops[a.type].compare_greater_than(a, b, pos);

	INVALID_OPERATION;
}

/* or truthiness of values */
EM_API em_value_t em_value_compare_or(em_value_t a, em_value_t b, em_pos_t *pos) {

	return EM_VALUE_INT(ops[a.type].is_true(a, pos).value.te_inttype || ops[b.type].is_true(b, pos).value.te_inttype);
}

/* and truthiness of values */
EM_API em_value_t em_value_compare_and(em_value_t a, em_value_t b, em_pos_t *pos) {

	return EM_VALUE_INT(ops[a.type].is_true(a, pos).value.te_inttype && ops[b.type].is_true(b, pos).value.te_inttype);
}

/* get hash value */
EM_API em_hash_t em_value_hash(em_value_t v, em_pos_t *pos) {

	if (ops[v.type].hash) return ops[v.type].hash(v, pos);

	return 0;
}

/* get value by key hash */
EM_API em_value_t em_value_get_by_hash(em_value_t v, em_hash_t hash, em_pos_t *pos) {

	if (ops[v.type].get_by_hash) return ops[v.type].get_by_hash(v, hash, pos);

	INVALID_OPERATION;
}

/* get value by index value */
EM_API em_value_t em_value_get_by_index(em_value_t v, em_value_t i, em_pos_t *pos) {

	if (ops[v.type].get_by_index) return ops[v.type].get_by_index(v, i, pos);

	INVALID_OPERATION;
}

/* set value by key hash */
EM_API em_result_t em_value_set_by_hash(em_value_t a, em_hash_t hash, em_value_t b, em_pos_t *pos) {

	if (ops[a.type].set_by_hash) return ops[a.type].set_by_hash(a, hash, b, pos);

	INVALID_OPERATION_RETURN(EM_RESULT_FAILURE);
}

/* set value by index */
EM_API em_result_t em_value_set_by_index(em_value_t a, em_value_t i, em_value_t b, em_pos_t *pos) {

	if (ops[a.type].set_by_index) return ops[a.type].set_by_index(a, i, b, pos);

	INVALID_OPERATION_RETURN(EM_RESULT_FAILURE);
}

/* call value */
EM_API em_value_t em_value_call(struct em_context *context, em_value_t v, em_value_t *args, size_t nargs, em_pos_t *pos) {

	if (ops[v.type].call) return ops[v.type].call(context, v, args, nargs, pos);

	INVALID_OPERATION;
}

/* get value length */
EM_API em_value_t em_value_length_of(em_value_t v, em_pos_t *pos) {

	if (ops[v.type].length_of) return ops[v.type].length_of(v, pos);

	INVALID_OPERATION;
}

/* get string representation of value */
EM_API em_value_t em_value_to_string(em_value_t v, em_pos_t *pos) {

	if (ops[v.type].to_string) return ops[v.type].to_string(v, pos);

	return em_string_new_from_utf8("(None)", 6);
}

/* log value */
EM_API void em_value_log(em_value_t v) {

	em_value_t string = em_value_to_string(v, NULL); /* TODO: Figure out what to do about NULL position */

	static char stringbuf[1024];
	em_wchar_to_utf8(stringbuf, 1024, EM_STRING(EM_OBJECT_FROM_VALUE(string))->data);
	em_log_info("%s", stringbuf);

	if (v.type != EM_VALUE_TYPE_OBJECT ||
	    string.value.t_voidp != v.value.t_voidp)
		em_value_delete(string);
}
