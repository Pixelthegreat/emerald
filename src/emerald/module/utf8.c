/*
 * Copyright 2026, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <emerald/core.h>
#include <emerald/utf8.h>
#include <emerald/wchar.h>
#include <emerald/util.h>
#include <emerald/string.h>
#include <emerald/map.h>
#include <emerald/context.h>
#include <emerald/module/array.h>
#include <emerald/module/utf8.h>

/* utf8 module */
static em_result_t initialize(em_context_t *context, em_value_t map);

em_module_t em_module_utf8 = {
	.initialize = initialize,
};

/* encode integer to bytes */
static em_value_t utf8_encodeInteger(em_context_t *context, em_value_t *args, size_t nargs, em_pos_t *pos) {

	em_value_t value;
	em_inttype_t code;

	if (em_util_parse_args(pos, args, nargs, "bi", &value, &code) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	em_byte_array_t *array = EM_BYTE_ARRAY(EM_OBJECT_FROM_VALUE(value));
	if (array->size < 4 ||
	    (array->mode != EM_BYTE_ARRAY_MODE_UNSIGNED_CHAR &&
	     array->mode != EM_BYTE_ARRAY_MODE_CHAR)) {

		em_log_runtime_error(pos, "Invalid arguments");
		return EM_VALUE_FAIL;
	}

	em_ssize_t len = em_utf8_putch((char *)array->data, (int)code);
	if (len < 1 || len > 4) {

		em_log_runtime_error(pos, "Invalid Unicode code point");
		return EM_VALUE_FAIL;
	}
	return EM_VALUE_INT((em_inttype_t)len);
}

/* decode bytes to integer */
static em_value_t utf8_decodeInteger(em_context_t *context, em_value_t *args, size_t nargs, em_pos_t *pos) {

	em_value_t value;

	if (em_util_parse_args(pos, args, nargs, "b", &value) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	char bytes[4];
	memset(bytes, 0, 4);

	em_byte_array_t *array = EM_BYTE_ARRAY(EM_OBJECT_FROM_VALUE(value));
	if (array->size < 4 ||
	    (array->mode != EM_BYTE_ARRAY_MODE_UNSIGNED_CHAR &&
	     array->mode != EM_BYTE_ARRAY_MODE_CHAR)) {

		em_log_runtime_error(pos, "Invalid arguments");
		return EM_VALUE_FAIL;
	}
	size_t tocopy = array->size < 4? array->size: 4;

	memcpy(bytes, array->data, tocopy);

	/* decode bytes */
	em_ssize_t nbytes;
	int code = em_utf8_getch(bytes, &nbytes);

	if (code < 0) {

		em_log_runtime_error(pos, "Invalid UTF-8 bytes");
		return EM_VALUE_FAIL;
	}
	return EM_VALUE_INT((em_inttype_t)code);
}

/* encode string */
static em_value_t utf8_encode(em_context_t *context, em_value_t *args, size_t nargs, em_pos_t *pos) {

	em_value_t value;
	const em_wchar_t *string;

	if (em_util_parse_args(pos, args, nargs, "bW", &value, &string) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	em_byte_array_t *array = EM_BYTE_ARRAY(EM_OBJECT_FROM_VALUE(value));

	if (array->mode != EM_BYTE_ARRAY_MODE_UNSIGNED_CHAR &&
	    array->mode != EM_BYTE_ARRAY_MODE_CHAR) {

		em_log_runtime_error(pos, "Invalid arguments");
		return EM_VALUE_FAIL;
	}

	/* encode bytes */
	size_t i = 0, chsize = 0;
	for (; i < array->size && !EM_WCZ(*string); i += chsize, string++) {

		int code = EM_WC2INT(*string);
		em_ssize_t nbytes = em_utf8_getchlen(code);

		if (nbytes < 0 || nbytes > 4) {

			em_log_runtime_error(pos, "Invalid Unicode code point");
			return EM_VALUE_FAIL;
		}
		chsize = (size_t)nbytes;
		if (chsize + i > array->size) break;

		em_utf8_putch((char *)array->data + i, code);
	}
	return EM_VALUE_INT((em_inttype_t)i);
}

/* decode string */
static em_value_t utf8_decode(em_context_t *context, em_value_t *args, size_t nargs, em_pos_t *pos) {

	em_value_t svalue, avalue;

	if (em_util_parse_args(pos, args, nargs, "wb", &svalue, &avalue) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	em_string_t *string = EM_STRING(EM_OBJECT_FROM_VALUE(svalue));
	em_byte_array_t *array = EM_BYTE_ARRAY(EM_OBJECT_FROM_VALUE(avalue));

	if (array->mode != EM_BYTE_ARRAY_MODE_UNSIGNED_CHAR &&
	    array->mode != EM_BYTE_ARRAY_MODE_CHAR) {

		em_log_runtime_error(pos, "Invalid arguments");
		return EM_VALUE_FAIL;
	}

	/* decode bytes */
	size_t i = 0;
	size_t nbytes = 0;
	size_t chsize = 0;
	for (; i < string->length && nbytes < array->size; i++, nbytes += chsize) {

		char bytes[4];
		memset(bytes, 0, 4);

		size_t tocopy = (array->size - nbytes) < 4? (array->size - nbytes): 4;
		memcpy(bytes, array->data + nbytes, tocopy);

		em_ssize_t nchbytes;
		int code = em_utf8_getch(bytes, &nchbytes);

		if (code < 0) {

			em_log_runtime_error(pos, "Invalid UTF-8 bytes");
			return EM_VALUE_FAIL;
		}
		chsize = (size_t)nchbytes;

		string->data[i] = EM_INT2WC(code);
	}

	return EM_VALUE_INT((em_inttype_t)nbytes);
}

/* validate utf8 bytes */
static em_value_t utf8_validateBytes(em_context_t *context, em_value_t *args, size_t nargs, em_pos_t *pos) {

	em_value_t value;

	if (em_util_parse_args(pos, args, nargs, "b", &value) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	em_byte_array_t *array = EM_BYTE_ARRAY(EM_OBJECT_FROM_VALUE(value));

	if (array->mode != EM_BYTE_ARRAY_MODE_UNSIGNED_CHAR &&
	    array->mode != EM_BYTE_ARRAY_MODE_CHAR) {

		em_log_runtime_error(pos, "Invalid arguments");
		return EM_VALUE_FAIL;
	}

	/* validate bytes */
	size_t nbytes = 0;
	while (nbytes < array->size) {

		char bytes[4];
		memset(bytes, 0, 4);

		size_t tocopy = (array->size - nbytes) < 4? (array->size - nbytes): 4;
		memcpy(bytes, array->data + nbytes, tocopy);

		em_ssize_t len;
		int code = em_utf8_getch(bytes, &len);

		if (code < 0) return EM_VALUE_FALSE;
		nbytes += (size_t)len;
	}
	return EM_VALUE_TRUE;
}

/* initialize module */
static em_result_t initialize(em_context_t *context, em_value_t map) {

	em_value_t mod = em_map_new();
	em_util_set_value(map, "__module_utf8", mod);

	/* functions */
	em_util_set_function(mod, "encodeInteger", utf8_encodeInteger);
	em_util_set_function(mod, "decodeInteger", utf8_decodeInteger);
	em_util_set_function(mod, "encode", utf8_encode);
	em_util_set_function(mod, "decode", utf8_decode);
	em_util_set_function(mod, "validateBytes", utf8_validateBytes);

	return EM_RESULT_SUCCESS;
}
