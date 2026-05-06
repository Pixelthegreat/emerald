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

	em_floattype_t ft_value;

	if (em_util_parse_args(pos, args, nargs, "N", &ft_value) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	return EM_VALUE_FLOAT(floor(ft_value));
}

/* ceil value */
static em_value_t math_ceil(em_context_t *context, em_value_t *args, size_t nargs, em_pos_t *pos) {

	em_floattype_t ft_value;

	if (em_util_parse_args(pos, args, nargs, "N", &ft_value) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	return EM_VALUE_FLOAT(ceil(ft_value));
}

/* round value */
static em_value_t math_round(em_context_t *context, em_value_t *args, size_t nargs, em_pos_t *pos) {

	em_floattype_t ft_value;

	if (em_util_parse_args(pos, args, nargs, "N", &ft_value) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	return EM_VALUE_FLOAT(round(ft_value));
}

/* sine of value */
static em_value_t math_sin(em_context_t *context, em_value_t *args, size_t nargs, em_pos_t *pos) {

	em_floattype_t ft_value;

	if (em_util_parse_args(pos, args, nargs, "N", &ft_value) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	return EM_VALUE_FLOAT(sin(ft_value));
}

/* cosine of value */
static em_value_t math_cos(em_context_t *context, em_value_t *args, size_t nargs, em_pos_t *pos) {

	em_floattype_t ft_value;

	if (em_util_parse_args(pos, args, nargs, "N", &ft_value) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	return EM_VALUE_FLOAT(cos(ft_value));
}

/* tangent of value */
static em_value_t math_tan(em_context_t *context, em_value_t *args, size_t nargs, em_pos_t *pos) {

	em_floattype_t ft_value;

	if (em_util_parse_args(pos, args, nargs, "N", &ft_value) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	return EM_VALUE_FLOAT(tan(ft_value));
}

/* arc sine of value */
static em_value_t math_asin(em_context_t *context, em_value_t *args, size_t nargs, em_pos_t *pos) {

	em_floattype_t ft_value;

	if (em_util_parse_args(pos, args, nargs, "N", &ft_value) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	return EM_VALUE_FLOAT(asin(ft_value));
}

/* arc cosine of value */
static em_value_t math_acos(em_context_t *context, em_value_t *args, size_t nargs, em_pos_t *pos) {

	em_floattype_t ft_value;

	if (em_util_parse_args(pos, args, nargs, "N", &ft_value) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	return EM_VALUE_FLOAT(acos(ft_value));
}

/* arc tangent of value */
static em_value_t math_atan(em_context_t *context, em_value_t *args, size_t nargs, em_pos_t *pos) {

	em_floattype_t ft_value;

	if (em_util_parse_args(pos, args, nargs, "N", &ft_value) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	return EM_VALUE_FLOAT(atan(ft_value));
}

/* minimum value */
static em_value_t math_min(em_context_t *context, em_value_t *args, size_t nargs, em_pos_t *pos) {

	em_floattype_t ft_a, ft_b;

	if (em_util_parse_args(pos, args, nargs, "NN", &ft_a, &ft_b) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	return EM_VALUE_FLOAT(EM_MIN(ft_a, ft_b));
}

/* maximum value */
static em_value_t math_max(em_context_t *context, em_value_t *args, size_t nargs, em_pos_t *pos) {

	em_floattype_t ft_a, ft_b;

	if (em_util_parse_args(pos, args, nargs, "NN", &ft_a, &ft_b) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	return EM_VALUE_FLOAT(EM_MAX(ft_a, ft_b));
}

/* clamp value */
static em_value_t math_clamp(em_context_t *context, em_value_t *args, size_t nargs, em_pos_t *pos) {

	em_floattype_t ft_v, ft_a, ft_b;

	if (em_util_parse_args(pos, args, nargs, "NNN", &ft_v, &ft_a, &ft_b) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	return EM_VALUE_FLOAT(EM_CLAMP(ft_v, ft_a, ft_b));
}

/* initialize module */
static em_result_t initialize(em_context_t *context, em_value_t map) {

	em_value_t mod = em_map_new();
	em_util_set_value(map, "__module_math", mod);

	/* constants */
	em_util_set_value(mod, "pi", EM_VALUE_FLOAT(3.141592653589793));
	em_util_set_value(mod, "tau", EM_VALUE_FLOAT(6.283185307179586));
	em_util_set_value(mod, "e", EM_VALUE_FLOAT(2.718281828459045));

	/* functions */
	em_util_set_function(mod, "floor", math_floor);
	em_util_set_function(mod, "ceil", math_ceil);
	em_util_set_function(mod, "round", math_round);
	em_util_set_function(mod, "sin", math_sin);
	em_util_set_function(mod, "cos", math_cos);
	em_util_set_function(mod, "tan", math_tan);
	em_util_set_function(mod, "asin", math_asin);
	em_util_set_function(mod, "acos", math_acos);
	em_util_set_function(mod, "atan", math_atan);
	em_util_set_function(mod, "min", math_min);
	em_util_set_function(mod, "max", math_max);
	em_util_set_function(mod, "clamp", math_clamp);

	return EM_RESULT_SUCCESS;
}
