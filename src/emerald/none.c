/*
 * Copyright 2026, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <emerald/core.h>
#include <emerald/string.h>
#include <emerald/none.h>

/* none type */
static em_value_t is_true(em_value_t v, em_pos_t *pos);
static em_value_t compare_equal(em_value_t a, em_value_t b, em_pos_t *pos);
static em_value_t to_string(em_value_t v, em_pos_t *pos);

static em_object_type_t type = {
	.is_true = is_true,
	.compare_equal = compare_equal,
	.to_string = to_string,
};

/* get truthiness of none */
static em_value_t is_true(em_value_t v, em_pos_t *pos) {

	return EM_VALUE_FALSE;
}

/* compare none */
static em_value_t compare_equal(em_value_t a, em_value_t b, em_pos_t *pos) {

	if (a.type != b.type) return EM_VALUE_FALSE;

	return (a.value.t_voidp == b.value.t_voidp)? EM_VALUE_TRUE: EM_VALUE_FALSE;
}

/* get string representation of none */
static em_value_t to_string(em_value_t v, em_pos_t *pos) {

	return em_string_new_from_utf8("none", 4);
}

/* create none */
EM_API em_value_t em_none_new(void) {

	return em_object_new(&type, sizeof(em_object_t));
}
