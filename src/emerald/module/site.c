/*
 * Copyright 2025, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <emerald/core.h>
#include <emerald/utf8.h>
#include <emerald/wchar.h>
#include <emerald/value.h>
#include <emerald/none.h>
#include <emerald/util.h>
#include <emerald/list.h>
#include <emerald/string.h>
#include <emerald/module/site.h>

/* site module */
static em_result_t initialize(em_context_t *context, em_value_t map);

em_module_t em_module_site = {
	.initialize = initialize,
};

/* get length of value */
static em_value_t site_lengthOf(em_context_t *context, em_value_t *args, size_t nargs, em_pos_t *pos) {

	em_value_t value;

	if (em_util_parse_args(pos, args, nargs, "v", &value) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	return em_value_length_of(value, pos);
}

/* get string representation of value */
static em_value_t site_toString(em_context_t *context, em_value_t *args, size_t nargs, em_pos_t *pos) {

	em_value_t value;

	if (em_util_parse_args(pos, args, nargs, "v", &value) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	return em_value_to_string(value, pos);
}

/* append item to list */
static em_value_t site_append(em_context_t *context, em_value_t *args, size_t nargs, em_pos_t *pos) {

	em_value_t list, value;

	if (em_util_parse_args(pos, args, nargs, "lv", &list, &value) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	em_list_append(list, value);
	return value;
}

/* exit interpreter */
static em_value_t site_exit(em_context_t *context, em_value_t *args, size_t nargs, em_pos_t *pos) {

	context->pass = EM_VALUE_INT(0);
	if (nargs && em_util_parse_args(pos, args, nargs, "i", &context->pass.value.te_inttype) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	em_log_raise("SystemExit", pos, "Exited");
	return EM_VALUE_FAIL;
}

/* print without newline */
static em_value_t site_print(em_context_t *context, em_value_t *args, size_t nargs, em_pos_t *pos) {

	em_value_t value;

	if (em_util_parse_args(pos, args, nargs, "v", &value) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	em_value_t string = em_value_to_string(value, pos);
	if (!EM_VALUE_OK(string))
		return EM_VALUE_FAIL;

	em_string_t *pstring = EM_STRING(EM_OBJECT_FROM_VALUE(string));
	em_wchar_write(stdout, pstring->data, pstring->length);
	fflush(stdout);

	if (!em_value_is(value, string))
		em_value_delete(string);
	return em_none;
}

/* print with newline */
static em_value_t site_println(em_context_t *context, em_value_t *args, size_t nargs, em_pos_t *pos) {

	em_value_t value;

	if (em_util_parse_args(pos, args, nargs, "v", &value) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	em_value_t string = em_value_to_string(value, pos);
	if (!EM_VALUE_OK(string))
		return EM_VALUE_FAIL;

	em_string_t *pstring = EM_STRING(EM_OBJECT_FROM_VALUE(string));
	em_wchar_write(stdout, pstring->data, pstring->length);
	fprintf(stdout, "\n");
	fflush(stdout);

	if (!em_value_is(value, string))
		em_value_delete(string);
	return em_none;
}

/* initialize module */
static em_result_t initialize(em_context_t *context, em_value_t map) {

	/* common functions */
	em_util_set_function(map, "lengthOf", site_lengthOf);
	em_util_set_function(map, "toString", site_toString);
	em_util_set_function(map, "append", site_append);
	em_util_set_function(map, "exit", site_exit);
	em_util_set_function(map, "print", site_print);
	em_util_set_function(map, "println", site_println);

	/* common variables */
	em_util_set_value(map, "true", EM_VALUE_TRUE);
	em_util_set_value(map, "false", EM_VALUE_FALSE);
	em_util_set_value(map, "none", em_none);

	/* create argument list */
	em_value_t argv;
	if (context->argv) {

		size_t nargs = 0;
		while (context->argv[nargs]) nargs++;

		argv = em_list_new(nargs);
		for (size_t i = 0; i < nargs; i++) {

			em_value_t string = em_string_new_from_utf8(context->argv[i], em_utf8_strlen(context->argv[i]));
			em_list_append(argv, string);
		}
	}
	else argv = em_list_new(0);
	em_util_set_value(map, "argv", argv);

	return EM_RESULT_SUCCESS;
}
