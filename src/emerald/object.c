/*
 * Copyright 2025, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <emerald/core.h>
#include <emerald/string.h>
#include <emerald/object.h>

#define INVALID_OPERATION ({\
		em_log_runtime_error(pos, "Invalid operation");\
		return EM_VALUE_FAIL;\
	})

/* create object */
EM_API em_value_t em_object_new(em_object_type_t *type, size_t size) {

	em_object_t *object = EM_OBJECT(em_refobj_new(&em_reflist_object, size, EM_CLEANUP_MODE_IMMEDIATE));
	if (!object) return EM_VALUE_FAIL;

	object->type = type;
	return EM_OBJECT_AS_VALUE(object);
}

/* get truthiness of object */
EM_API em_value_t em_object_is_true(em_value_t v, em_pos_t *pos) {

	em_object_t *object = EM_OBJECT_FROM_VALUE(v);
	if (!object->type->is_true) return EM_VALUE_TRUE;

	return object->type->is_true(v, pos);
}

/* add objects */
EM_API em_value_t em_object_add(em_value_t a, em_value_t b, em_pos_t *pos) {

	em_object_t *object = EM_OBJECT_FROM_VALUE(a);
	em_object_t *other = EM_OBJECT_FROM_VALUE(b);

	if (!object->type->add) INVALID_OPERATION;

	return object->type->add(a, b, pos);
}

/* subtract objects */
EM_API em_value_t em_object_subtract(em_value_t a, em_value_t b, em_pos_t *pos) {

	em_object_t *object = EM_OBJECT_FROM_VALUE(a);
	em_object_t *other = EM_OBJECT_FROM_VALUE(b);

	if (!object->type->subtract) INVALID_OPERATION;

	return object->type->subtract(a, b, pos);
}

/* multiply objects */
EM_API em_value_t em_object_multiply(em_value_t a, em_value_t b, em_pos_t *pos) {

	em_object_t *object = EM_OBJECT_FROM_VALUE(a);
	em_object_t *other = EM_OBJECT_FROM_VALUE(b);

	if (!object->type->multiply) INVALID_OPERATION;

	return object->type->multiply(a, b, pos);
}

/* divide objects */
EM_API em_value_t em_object_divide(em_value_t a, em_value_t b, em_pos_t *pos) {

	em_object_t *object = EM_OBJECT_FROM_VALUE(a);
	em_object_t *other = EM_OBJECT_FROM_VALUE(b);

	if (!object->type->divide) INVALID_OPERATION;

	return object->type->divide(a, b, pos);
}

/* or objects */
EM_API em_value_t em_object_or(em_value_t a, em_value_t b, em_pos_t *pos) {

	em_object_t *object = EM_OBJECT_FROM_VALUE(a);
	em_object_t *other = EM_OBJECT_FROM_VALUE(b);

	if (!object->type->or) INVALID_OPERATION;

	return object->type->or(a, b, pos);
}

/* and objects */
EM_API em_value_t em_object_and(em_value_t a, em_value_t b, em_pos_t *pos) {

	em_object_t *object = EM_OBJECT_FROM_VALUE(a);
	em_object_t *other = EM_OBJECT_FROM_VALUE(b);

	if (!object->type->and) INVALID_OPERATION;

	return object->type->and(a, b, pos);
}

/* shift left */
EM_API em_value_t em_object_shift_left(em_value_t a, em_value_t b, em_pos_t *pos) {

	em_object_t *object = EM_OBJECT_FROM_VALUE(a);
	em_object_t *other = EM_OBJECT_FROM_VALUE(b);

	if (!object->type->shift_left) INVALID_OPERATION;

	return object->type->shift_left(a, b, pos);
}

/* shift right */
EM_API em_value_t em_object_shift_right(em_value_t a, em_value_t b, em_pos_t *pos) {

	em_object_t *object = EM_OBJECT_FROM_VALUE(a);
	em_object_t *other = EM_OBJECT_FROM_VALUE(b);

	if (!object->type->shift_right) INVALID_OPERATION;

	return object->type->shift_right(a, b, pos);
}

/* compare if objects are equal */
EM_API em_value_t em_object_compare_equal(em_value_t a, em_value_t b, em_pos_t *pos) {

	em_object_t *object = EM_OBJECT_FROM_VALUE(a);
	em_object_t *other = EM_OBJECT_FROM_VALUE(b);

	if (!object->type->compare_equal) return EM_VALUE_FALSE;

	return object->type->compare_equal(a, b, pos);
}

/* compare if object is less than other */
EM_API em_value_t em_object_compare_less_than(em_value_t a, em_value_t b, em_pos_t *pos) {

	em_object_t *object = EM_OBJECT_FROM_VALUE(a);
	em_object_t *other = EM_OBJECT_FROM_VALUE(b);

	if (!object->type->compare_less_than) INVALID_OPERATION;

	return object->type->compare_less_than(a, b, pos);
}

/* compare if object is greater than other */
EM_API em_value_t em_object_compare_greater_than(em_value_t a, em_value_t b, em_pos_t *pos) {

	em_object_t *object = EM_OBJECT_FROM_VALUE(a);
	em_object_t *other = EM_OBJECT_FROM_VALUE(b);

	if (!object->type->compare_greater_than) INVALID_OPERATION;

	return object->type->compare_greater_than(a, b, pos);
}

/* get string representation of object */
EM_API em_value_t em_object_to_string(em_value_t v, em_pos_t *pos) {

	em_object_t *object = EM_OBJECT_FROM_VALUE(v);
	if (!object->type->to_string) return em_string_new_from_utf8("(None)", 6);

	return object->type->to_string(v, pos);
}
