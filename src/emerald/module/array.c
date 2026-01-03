/*
 * Copyright 2026, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <emerald/core.h>
#include <emerald/none.h>
#include <emerald/util.h>
#include <emerald/string.h>
#include <emerald/context.h>
#include <emerald/module/array.h>

/* mode sizes */
static size_t modesizes[EM_BYTE_ARRAY_MODE_COUNT] = {
	1, 1, 2, 2, 4, 4, sizeof(em_inttype_t),
};

/* array module */
static em_result_t initialize(em_context_t *context, em_value_t map);

em_module_t em_module_array = {
	.initialize = initialize,
};

/* create byte array */
static em_value_t array_Array(em_context_t *context, em_value_t *args, size_t nargs, em_pos_t *pos) {

	em_inttype_t size, mode;

	if (em_util_parse_args(pos, args, nargs, "ii", &size, &mode) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	if (mode < 0 || mode >= EM_BYTE_ARRAY_MODE_COUNT) {

		em_log_runtime_error(pos, "Invalid byte array mode");
		return EM_VALUE_FAIL;
	}
	else if (size < 1) {

		em_log_runtime_error(pos, "Invalid byte array size");
		return EM_VALUE_FAIL;
	}

	return em_byte_array_new((size_t)size, (em_byte_array_mode_t)mode);
}

/* initialize module */
static em_result_t initialize(em_context_t *context, em_value_t map) {

	em_value_t mod = em_map_new();
	em_util_set_value(map, "__module_array", mod);

	/* modes */
	em_util_set_value(mod, "char", EM_VALUE_INT(EM_BYTE_ARRAY_MODE_CHAR));
	em_util_set_value(mod, "unsignedChar", EM_VALUE_INT(EM_BYTE_ARRAY_MODE_UNSIGNED_CHAR));
	em_util_set_value(mod, "short", EM_VALUE_INT(EM_BYTE_ARRAY_MODE_SHORT));
	em_util_set_value(mod, "unsignedShort", EM_VALUE_INT(EM_BYTE_ARRAY_MODE_UNSIGNED_SHORT));
	em_util_set_value(mod, "int", EM_VALUE_INT(EM_BYTE_ARRAY_MODE_INT));
	em_util_set_value(mod, "unsignedInt", EM_VALUE_INT(EM_BYTE_ARRAY_MODE_UNSIGNED_INT));
	em_util_set_value(mod, "long", EM_VALUE_INT(EM_BYTE_ARRAY_MODE_LONG));

	/* functions */
	em_util_set_function(mod, "Array", array_Array);

	return EM_RESULT_SUCCESS;
}

/* type */
static em_value_t get_by_index(em_value_t v, em_value_t i, em_pos_t *pos);
static em_result_t set_by_index(em_value_t a, em_value_t i, em_value_t b, em_pos_t *pos);
static em_value_t length_of(em_value_t v, em_pos_t *pos);
static em_value_t to_string(em_value_t v, em_pos_t *pos);

static em_object_type_t type = {
	.get_by_index = get_by_index,
	.set_by_index = set_by_index,
	.length_of = length_of,
	.to_string = to_string,
};

/* get value by index */
static em_value_t get_by_index(em_value_t v, em_value_t i, em_pos_t *pos) {

	em_byte_array_t *array = EM_BYTE_ARRAY(EM_OBJECT_FROM_VALUE(v));

	if (i.type != EM_VALUE_TYPE_INT ||
	    i.value.te_inttype < -(em_inttype_t)array->size ||
	    i.value.te_inttype >= (em_inttype_t)array->size)
		return EM_VALUE_FAIL;

	return EM_VALUE_INT(em_byte_array_get(v, (em_ssize_t)i.value.te_inttype));
}

/* set value by index */
static em_result_t set_by_index(em_value_t a, em_value_t i, em_value_t b, em_pos_t *pos) {

	em_byte_array_t *array = EM_BYTE_ARRAY(EM_OBJECT_FROM_VALUE(a));

	if (b.type != EM_VALUE_TYPE_INT) {

		em_log_runtime_error(pos, "Invalid operation");
		return EM_RESULT_FAILURE;
	}

	if (i.type != EM_VALUE_TYPE_INT ||
	    i.value.te_inttype < -(em_inttype_t)array->size ||
	    i.value.te_inttype >= (em_inttype_t)array->size)
		return EM_RESULT_FAILURE;

	em_byte_array_set(a, (em_ssize_t)i.value.te_inttype, b.value.te_inttype);
	return EM_RESULT_SUCCESS;
}

/* get length of byte array */
static em_value_t length_of(em_value_t v, em_pos_t *pos) {

	em_byte_array_t *array = EM_BYTE_ARRAY(EM_OBJECT_FROM_VALUE(v));
	return EM_VALUE_INT((em_inttype_t)array->size);
}

/* get string representation of byte array */
static em_value_t to_string(em_value_t v, em_pos_t *pos) {

	em_byte_array_t *array = EM_BYTE_ARRAY(EM_OBJECT_FROM_VALUE(v));

	char buf[128];
	snprintf(buf, 128, "<Byte array of size %zu>", array->size);

	return em_string_new_from_utf8(buf, strlen(buf));
}

/* create byte array */
EM_API em_value_t em_byte_array_new(size_t size, em_byte_array_mode_t mode) {

	size_t full_size = size * modesizes[mode];

	em_value_t value = em_object_new(&type, sizeof(em_byte_array_t) + full_size);
	em_byte_array_t *array = EM_BYTE_ARRAY(EM_OBJECT_FROM_VALUE(value));

	array->size = size;
	array->mode = mode;
	memset(array->data, 0, full_size);

	return value;
}

/* set value in byte array */
EM_API void em_byte_array_set(em_value_t object, em_ssize_t index, em_inttype_t value) {

	em_byte_array_t *array = EM_BYTE_ARRAY(EM_OBJECT_FROM_VALUE(object));
	if (index < 0) index = (em_ssize_t)array->size - index;

	if (index < 0 || index >= (em_ssize_t)array->size)
		return;

	/* set value */
	switch (array->mode) {
		/* int8 */
		case EM_BYTE_ARRAY_MODE_CHAR:
			*((int8_t *)array->data + index) = (int8_t)value;
			break;
		/* uint8 */
		case EM_BYTE_ARRAY_MODE_UNSIGNED_CHAR:
			*(array->data + index) = (uint8_t)value;
			break;
		/* int16 */
		case EM_BYTE_ARRAY_MODE_SHORT:
			*((int16_t *)array->data + index) = (int16_t)value;
			break;
		/* uint16 */
		case EM_BYTE_ARRAY_MODE_UNSIGNED_SHORT:
			*((uint16_t *)array->data + index) = (uint16_t)value;
			break;
		/* int32 */
		case EM_BYTE_ARRAY_MODE_INT:
			*((int32_t *)array->data + index) = (int32_t)value;
			break;
		/* uint32 */
		case EM_BYTE_ARRAY_MODE_UNSIGNED_INT:
			*((uint32_t *)array->data + index) = (uint32_t)value;
			break;
		/* inttype */
		case EM_BYTE_ARRAY_MODE_LONG:
			*((em_inttype_t *)array->data + index) = value;
			break;
	}
}

/* get value from byte array */
EM_API em_inttype_t em_byte_array_get(em_value_t object, em_ssize_t index) {

	em_byte_array_t *array = EM_BYTE_ARRAY(EM_OBJECT_FROM_VALUE(object));
	if (index < 0) index = (em_ssize_t)array->size - index;

	if (index < 0 || index >= (em_ssize_t)array->size)
		return 0;

	/* get value */
	switch (array->mode) {
		case EM_BYTE_ARRAY_MODE_CHAR:
			return (em_inttype_t)*((int8_t *)array->data + index);
		case EM_BYTE_ARRAY_MODE_UNSIGNED_CHAR:
			return (em_inttype_t)*(array->data + index);
		case EM_BYTE_ARRAY_MODE_SHORT:
			return (em_inttype_t)*((int16_t *)array->data + index);
		case EM_BYTE_ARRAY_MODE_UNSIGNED_SHORT:
			return (em_inttype_t)*((uint16_t *)array->data + index);
		case EM_BYTE_ARRAY_MODE_INT:
			return (em_inttype_t)*((int32_t *)array->data + index);
		case EM_BYTE_ARRAY_MODE_UNSIGNED_INT:
			return (em_inttype_t)*((uint32_t *)array->data + index);
		case EM_BYTE_ARRAY_MODE_LONG:
			return *((em_inttype_t *)array->data + index);
		default:
			return 0;
	}
}

/* determine if value is byte array */
EM_API em_bool_t em_is_byte_array(em_value_t v) {

	return v.type == EM_VALUE_TYPE_OBJECT && EM_OBJECT_FROM_VALUE(v)->type == &type;
}
