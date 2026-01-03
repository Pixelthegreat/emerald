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
#include <emerald/hash.h>
#include <emerald/string.h>
#include <emerald/list.h>
#include <emerald/class.h>
#include <emerald/none.h>
#include <emerald/module/array.h>
#include <emerald/util.h>

#define INVALID_ARGUMENTS ({\
		em_log_runtime_error(pos, "Invalid arguments");\
		return EM_RESULT_FAILURE;\
	})

/* set value with utf8 name */
EM_API void em_util_set_value(em_value_t map, const char *name, em_value_t value) {

	em_map_set(map, em_utf8_strhash(name), value);
}

/* get value with utf8 name */
EM_API em_value_t em_util_get_value(em_value_t map, const char *name) {

	return em_map_get(map, em_utf8_strhash(name));
}

/* set value with utf8 name to utf8 string */
EM_API void em_util_set_string(em_value_t map, const char *name, const char *value) {

	em_util_set_value(map, name, em_string_new_from_utf8(value, em_utf8_strlen(value)));
}

/* builtin function shorthand */
EM_API void em_util_set_function(em_value_t map, const char *name, em_builtin_function_handler_t function) {

	em_map_set(map, em_utf8_strhash(name), em_builtin_function_new(name, function));
}

/* set value in class */
EM_API void em_util_set_class_value(em_value_t cls, const char *name, em_value_t value) {

	em_map_set(EM_CLASS(EM_OBJECT_FROM_VALUE(cls))->map, em_utf8_strhash(name), value);
}

/* set method in class */
EM_API void em_util_set_class_method(em_value_t cls, const char *name, em_builtin_function_handler_t function) {

	em_map_set(EM_CLASS(EM_OBJECT_FROM_VALUE(cls))->map, em_utf8_strhash(name), em_builtin_function_new(name, function));
}

/* parse arguments with variadic list */
EM_API em_result_t em_util_parse_vargs(em_pos_t *pos, em_value_t *args, size_t nargs, const char *format, va_list vargs) {

	size_t i = 0;
	char c = 0;
	char rc = 0; /* repeat character */
	void *pointer = NULL; /* argument pointer */
	em_bool_t optional = EM_FALSE;

	while (*format && i < nargs) {

		if (rc || *format == '*')
			rc = c;
		else if (*format == '~') {

			format++;
			optional = EM_TRUE;
			continue;
		}
		else {
			
			c = *format++;
			pointer = va_arg(vargs, void *);
		}
		em_value_t arg = args[i++];

		em_bool_t none_rule = EM_TRUE;
		if (optional && em_value_is(em_none, arg))
			none_rule = EM_FALSE;
		optional = EM_FALSE;

		switch (c) {

			/* any value */
			case 'v':
				if (rc) break;

				em_value_t *pvalue = (em_value_t *)pointer;
				if (pvalue) *pvalue = arg;
				break;

			/* number */
			case 'n':
				if (none_rule &&
				    arg.type != EM_VALUE_TYPE_INT &&
				    arg.type != EM_VALUE_TYPE_FLOAT)
					INVALID_ARGUMENTS;
				if (rc) break;

				em_value_t *pnumber = (em_value_t *)pointer;
				if (pnumber) *pnumber = arg;
				break;

			/* integer */
			case 'i':
				if (arg.type != EM_VALUE_TYPE_INT)
					INVALID_ARGUMENTS;
				if (rc) break;

				em_inttype_t *pinteger = (em_inttype_t *)pointer;
				if (pinteger) *pinteger = arg.value.te_inttype;
				break;

			/* floating point */
			case 'f':
				if (arg.type != EM_VALUE_TYPE_FLOAT)
					INVALID_ARGUMENTS;
				if (rc) break;

				em_floattype_t *pfloat = (em_floattype_t *)pointer;
				if (pfloat) *pfloat = arg.value.te_floattype;
				break;

			/* object */
			case 'o':
				if (none_rule &&
				    arg.type != EM_VALUE_TYPE_OBJECT)
					INVALID_ARGUMENTS;
				if (rc) break;

				em_value_t *pobject = (em_value_t *)pointer;
				if (pobject) *pobject = arg;
				break;

			/* wide string object */
			case 'w':
				if (none_rule &&
				    !em_is_string(arg))
					INVALID_ARGUMENTS;
				if (rc) break;

				em_value_t *pstring = (em_value_t *)pointer;
				if (pstring) *pstring = arg;
				break;

			/* wide string data pointer */
			case 'W':
				if (!em_is_string(arg))
					INVALID_ARGUMENTS;
				if (rc) break;

				const em_wchar_t **pstringdata = (const em_wchar_t **)pointer;
				if (pstringdata) *pstringdata = EM_STRING(EM_OBJECT_FROM_VALUE(arg))->data;
				break;

			/* list */
			case 'l':
				if (none_rule &&
				    !em_is_list(arg))
					INVALID_ARGUMENTS;
				if (rc) break;

				em_value_t *plist = (em_value_t *)pointer;
				if (plist) *plist = arg;
				break;

			/* map */
			case 'm':
				if (none_rule &&
				    !em_is_map(arg))
					INVALID_ARGUMENTS;
				if (rc) break;

				em_value_t *pmap = (em_value_t *)pointer;
				if (pmap) *pmap = arg;
				break;

			/* byte array */
			case 'b':
				if (none_rule &&
				    !em_is_byte_array(arg))
					INVALID_ARGUMENTS;
				if (rc) break;

				em_value_t *pbarray = (em_value_t *)pointer;
				if (pbarray) *pbarray = arg;
				break;

			/* error */
			default:
				em_log_fatal("Invalid format string");
				return EM_RESULT_FAILURE;
		}
	}

	/* invalid argument count */
	em_bool_t pass = EM_TRUE;
	if (*format && (*format == '*' || *(format+1) == '*'))
		pass = EM_FALSE;

	if ((i < nargs || *format) && pass) {

		if (i < nargs) em_log_runtime_error(pos, "Too many arguments");
		else em_log_runtime_error(pos, "Too few arguments");

		return EM_RESULT_FAILURE;
	}
	return EM_RESULT_SUCCESS;
}

/* parse arguments */
EM_API em_result_t em_util_parse_args(em_pos_t *pos, em_value_t *args, size_t nargs, const char *format, ...) {

	va_list vargs;
	va_start(vargs, format);
	em_result_t result = em_util_parse_vargs(pos, args, nargs, format, vargs);
	va_end(vargs);

	return result;
}
