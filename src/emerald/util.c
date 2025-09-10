/*
 * Copyright 2025, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <emerald/core.h>
#include <emerald/hash.h>
#include <emerald/string.h>
#include <emerald/list.h>
#include <emerald/map.h>
#include <emerald/util.h>

#define INVALID_ARGUMENTS ({\
		em_log_runtime_error(pos, "Invalid arguments");\
		return EM_RESULT_FAILURE;\
	})

/* set value with utf8 name */
EM_API void em_util_set_value(em_context_t *context, const char *name, em_value_t value) {

	em_context_set_value(context, em_utf8_strhash(name), value);
}

/* builtin function shorthand */
EM_API void em_util_set_function(em_context_t *context, const char *name, em_builtin_function_handler_t function) {

	em_context_set_value(context, em_utf8_strhash(name), em_builtin_function_new(name, function));
}

/* parse arguments with variadic list */
EM_API em_result_t em_util_parse_vargs(em_context_t *context, em_pos_t *pos, em_value_t *args, size_t nargs, const char *format, va_list vargs) {

	size_t i = 0;
	while (*format && i < nargs) {

		char c = *format++;
		em_value_t arg = args[i++];
		switch (c) {

			/* any value */
			case 'v':
				em_value_t *pvalue = va_arg(vargs, em_value_t *);
				*pvalue = arg;
				break;

			/* number */
			case 'n':
				if (arg.type != EM_VALUE_TYPE_INT &&
				    arg.type != EM_VALUE_TYPE_FLOAT)
					INVALID_ARGUMENTS;

				em_value_t *pnumber = va_arg(vargs, em_value_t *);
				*pnumber = arg;
				break;

			/* integer */
			case 'i':
				if (arg.type != EM_VALUE_TYPE_INT)
					INVALID_ARGUMENTS;

				em_inttype_t *pinteger = va_arg(vargs, em_inttype_t *);
				*pinteger = arg.value.te_inttype;
				break;

			/* floating point */
			case 'f':
				if (arg.type != EM_VALUE_TYPE_FLOAT)
					INVALID_ARGUMENTS;

				em_floattype_t *pfloat = va_arg(vargs, em_floattype_t *);
				*pfloat = arg.value.te_floattype;
				break;

			/* object */
			case 'o':
				if (arg.type != EM_VALUE_TYPE_OBJECT)
					INVALID_ARGUMENTS;

				em_value_t *pobject = va_arg(vargs, em_value_t *);
				*pobject = arg;
				break;

			/* wide string object */
			case 'w':
				if (!em_is_string(arg))
					INVALID_ARGUMENTS;

				em_value_t *pstring = va_arg(vargs, em_value_t *);
				*pstring = arg;
				break;

			/* wide string data pointer */
			case 'W':
				if (!em_is_string(arg))
					INVALID_ARGUMENTS;

				const em_wchar_t **pstringdata = va_arg(vargs, const em_wchar_t **);
				*pstringdata = EM_STRING(EM_OBJECT_FROM_VALUE(arg))->data;
				break;

			/* list */
			case 'l':
				if (!em_is_list(arg))
					INVALID_ARGUMENTS;

				em_value_t *plist = va_arg(vargs, em_value_t *);
				*plist = arg;
				break;

			/* map */
			case 'm':
				if (!em_is_map(arg))
					INVALID_ARGUMENTS;

				em_value_t *pmap = va_arg(vargs, em_value_t *);
				*pmap = arg;
				break;

			/* error */
			default:
				em_log_fatal("Invalid format string");
				return EM_RESULT_FAILURE;
		}
	}

	/* invalid argument count */
	if (i < nargs || *format) {

		if (i < nargs) em_log_runtime_error(pos, "Too many arguments");
		else em_log_runtime_error(pos, "Too few arguments");

		return EM_RESULT_FAILURE;
	}
	return EM_RESULT_SUCCESS;
}

/* parse arguments */
EM_API em_result_t em_util_parse_args(em_context_t *context, em_pos_t *pos, em_value_t *args, size_t nargs, const char *format, ...) {

	va_list vargs;
	va_start(vargs, format);
	em_result_t result = em_util_parse_vargs(context, pos, args, nargs, format, vargs);
	va_end(vargs);

	return result;
}
