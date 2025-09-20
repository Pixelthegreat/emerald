/*
 * Copyright 2025, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <emerald/core.h>
#include <emerald/value.h>
#include <emerald/none.h>
#include <emerald/util.h>
#include <emerald/list.h>
#include <emerald/module/site.h>

/* site module */
static em_result_t initialize(em_context_t *context);

em_module_t em_module_site = {
	.initialize = initialize,
};

/* get length of value */
static em_value_t site_lengthOf(em_context_t *context, em_value_t *args, size_t nargs, em_pos_t *pos) {

	em_value_t value;

	if (em_util_parse_args(context, pos, args, nargs, "v", &value) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	return em_value_length_of(value, pos);
}

/* get string representation of value */
static em_value_t site_toString(em_context_t *context, em_value_t *args, size_t nargs, em_pos_t *pos) {

	em_value_t value;

	if (em_util_parse_args(context, pos, args, nargs, "v", &value) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	return em_value_to_string(value, pos);
}

/* append item to list */
static em_value_t site_append(em_context_t *context, em_value_t *args, size_t nargs, em_pos_t *pos) {

	em_value_t list, value;

	if (em_util_parse_args(context, pos, args, nargs, "lv", &list, &value) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	em_list_append(list, value);
	return value;
}

/* exit interpreter */
static em_value_t site_exit(em_context_t *context, em_value_t *args, size_t nargs, em_pos_t *pos) {

	em_log_raise("SystemExit", pos, "Exited");
	return EM_VALUE_FAIL;
}

/* initialize module */
static em_result_t initialize(em_context_t *context) {

	/* common functions */
	em_util_set_function(context, "lengthOf", site_lengthOf);
	em_util_set_function(context, "toString", site_toString);
	em_util_set_function(context, "append", site_append);
	em_util_set_function(context, "exit", site_exit);

	/* common variables */
	em_util_set_value(context, "true", EM_VALUE_TRUE);
	em_util_set_value(context, "false", EM_VALUE_FALSE);
	em_util_set_value(context, "none", em_none);

	return EM_RESULT_SUCCESS;
}
