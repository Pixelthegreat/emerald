/*
 * Copyright 2025-2026, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <emerald/core.h>
#include <emerald/none.h>
#include <emerald/util.h>
#include <emerald/context.h>
#include <emerald/module/math.h>

/* math modules */
static em_result_t initialize(em_context_t *context, em_value_t map);

em_module_t em_module_math = {
	.initialize = initialize,
};

/* floor value */
static em_value_t math_floor(em_context_t *context, em_value_t *args, size_t nargs, em_pos_t *pos) {

	em_floattype_t value;

	if (em_util_parse_args(pos, args, nargs, "N", &value) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	return EM_VALUE_FLOAT(floor(value));
}

/* ceil value */
static em_value_t math_ceil(em_context_t *context, em_value_t *args, size_t nargs, em_pos_t *pos) {

	em_floattype_t value;

	if (em_util_parse_args(pos, args, nargs, "N", &value) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	return EM_VALUE_FLOAT(ceil(value));
}

/* round value */
static em_value_t math_round(em_context_t *context, em_value_t *args, size_t nargs, em_pos_t *pos) {

	em_floattype_t value;

	if (em_util_parse_args(pos, args, nargs, "N", &value) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	return EM_VALUE_FLOAT(round(value));
}

/* initialize module */
static em_result_t initialize(em_context_t *context, em_value_t map) {

	em_value_t mod = em_map_new();
	em_util_set_value(map, "__module_math", mod);

	/* functions */
	em_util_set_function(mod, "floor", math_floor);
	em_util_set_function(mod, "ceil", math_ceil);
	em_util_set_function(mod, "round", math_round);

	return EM_RESULT_SUCCESS;
}
