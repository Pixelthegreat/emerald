/*
 * Copyright 2026, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <emerald/core.h>
#include <emerald/value.h>
#include <emerald/string.h>
#include <emerald/util.h>
#include <emerald/module/string.h>

/* string module */
static em_result_t initialize(em_context_t *context, em_value_t map);

em_module_t em_module_string = {
	.initialize = initialize,
};

/* format a string */
static em_value_t string_format(em_context_t *context, em_value_t *args, size_t nargs, em_pos_t *pos) {

	const em_wchar_t *format;

	if (em_util_parse_args(pos, args, nargs, "Wv*", &format, NULL) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	/* get arguments as strings */
	em_value_t strings[EM_FUNCTION_MAX_ARGUMENTS];
	size_t nstrings = 0;

	for (size_t i = 1; i < nargs; i++) {

		em_value_t value = em_value_to_string(args[i], pos);
		if (!EM_VALUE_OK(value)) {

			for (size_t j = 0; j < nstrings; j++)
				em_value_delete(strings[j]);
			return EM_VALUE_FAIL;
		}
		strings[nstrings++] = value;
	}

	/* determine length of final string */
	size_t length = 0;
	const em_wchar_t *pformat = format;
	size_t index = 0;
	em_bool_t added = EM_FALSE, spec = EM_FALSE;

	while (!EM_WCZ(*pformat)) {

		em_wchar_t wc = *pformat++;
		int c = EM_WC2INT(wc);

		/* specifying an index */
		if (spec) {

			if (c == '{') {

				spec = EM_FALSE;
				length++;
			}
			else if (c >= '0' && c <= '9') {

				if (!added) index = 0;
				added = EM_TRUE;

				index = (index * 10) + (size_t)(c - '0');
			}
			else if (c == '}') {

				spec = EM_FALSE;
				added = EM_FALSE;

				if (index >= nstrings) {

					em_log_runtime_error(pos, "Invalid index");
					for (size_t i = 0; i < nstrings; i++)
						em_value_delete(strings[i]);
					return EM_VALUE_FAIL;
				}
				length += EM_STRING(EM_OBJECT_FROM_VALUE(strings[index]))->length;
				index++;
			}
		}
		else if (c == '{') spec = EM_TRUE;

		/* normal character */
		else length++;
	}

	if (spec) {

		em_log_runtime_error(pos, "Unclosed format specifier");
		for (size_t i = 0; i < nstrings; i++)
			em_value_delete(strings[i]);
		return EM_VALUE_FAIL;
	}

	/* create string */
	em_value_t string = em_string_new(length);
	em_wchar_t *buffer = EM_STRING(EM_OBJECT_FROM_VALUE(string))->data;
	buffer[length] = EM_INT2WC(0);

	size_t position = 0;

	index = 0;
	pformat = format; added = EM_FALSE; spec = EM_FALSE;

	while (!EM_WCZ(*pformat)) {

		em_wchar_t wc = *pformat++;
		int c = EM_WC2INT(wc);

		/* specifying an index */
		if (spec) {

			if (c == '{') {
				
				spec = EM_FALSE;
				buffer[position++] = EM_INT2WC('{');
			}
			else if (c >= '0' && c <= '9') {

				if (!added) index = 0;
				added = EM_TRUE;

				index = (index * 10) + (size_t)(c - '0');
			}
			else if (c == '}') {

				spec = EM_FALSE;
				added = EM_FALSE;

				em_string_t *mstring = EM_STRING(EM_OBJECT_FROM_VALUE(strings[index]));
				memcpy(buffer + position, mstring->data, mstring->length * sizeof(em_wchar_t));

				position += mstring->length;
				index++;
			}
		}
		else if (c == '{') spec = EM_TRUE;

		/* normal character */
		else buffer[position++] = wc;
	}

	for (size_t i = 0; i < nstrings; i++)
		em_value_delete(strings[i]);
	return string;
}

/* initialize module */
static em_result_t initialize(em_context_t *context, em_value_t map) {

	em_value_t mod = em_map_new();
	em_util_set_value(map, "__module_string", mod);

	/* functions */
	em_util_set_function(mod, "format", string_format);

	return EM_RESULT_SUCCESS;
}
