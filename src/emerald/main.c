/*
 * Copyright 2025, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <emerald/core.h>
#include <emerald/utf8.h>
#include <emerald/memory.h>
#include <emerald/refobj.h>
#include <emerald/none.h>
#include <emerald/class.h>
#include <emerald/util.h>
#include <emerald/context.h>
#include <emerald/map.h>
#include <emerald/string.h>
#include <emerald/main.h>

EM_API em_bool_t em_print_allocation_traffic;

em_reflist_t em_reflist_token = EM_REFLIST_INIT;
em_reflist_t em_reflist_node = EM_REFLIST_INIT;
em_reflist_t em_reflist_object = EM_REFLIST_INIT;
em_value_t em_none = EM_VALUE_FAIL;

em_value_t em_class_error = EM_VALUE_FAIL;
em_value_t em_class_syntax_error = EM_VALUE_FAIL;
em_value_t em_class_runtime_error = EM_VALUE_FAIL;
em_value_t em_class_system_break = EM_VALUE_FAIL;
em_value_t em_class_system_continue = EM_VALUE_FAIL;
em_value_t em_class_system_return = EM_VALUE_FAIL;
em_value_t em_class_system_exit = EM_VALUE_FAIL;

static em_init_flag_t init_flags;

/* initialize error */
static em_value_t error_initialize(em_context_t *context, em_value_t *args, size_t nargs, em_pos_t *pos) {

	em_value_t instance;
	em_value_t message;

	if (em_util_parse_args(pos, args, nargs, "mw", &instance, &message) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	em_util_set_value(instance, "_message", message);
	return em_none;
}

/* get string representation */
static em_value_t error_toString(em_context_t *context, em_value_t *args, size_t nargs, em_pos_t *pos) {

	em_value_t instance;

	if (em_util_parse_args(pos, args, nargs, "m", &instance) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	em_value_t message = em_util_get_value(instance, "_message");
	if (!EM_VALUE_OK(message))
		return em_string_new_from_utf8("", 0);
	return message;
}

/* create error class */
static em_value_t create_error_class(const char *name, em_value_t base) {

	em_value_t class = em_class_new(NULL, name, base, em_map_new());
	em_value_incref(class);

	em_util_set_class_method(class, "_initialize", error_initialize);
	em_util_set_class_method(class, "_toString", error_toString);

	return class;
}

/* instantiate an error (hacky method) */
EM_API void em_error_instantiate(em_value_t *value, em_value_t *cls, const char *message) {

	em_value_t instance = em_map_new();

	em_value_t to_string = em_util_get_value(EM_CLASS(EM_OBJECT_FROM_VALUE(*cls))->map, "_toString");

	size_t len = em_utf8_strlen(message);
	size_t slen = strlen(message);

	if (slen && message[slen-1] == '\n') {

		slen--;
		len--;
	}

	em_util_set_value(instance, "_class", *cls);
	em_util_set_value(instance, "_message", em_string_new_from_utf8(message, len));
	em_util_set_value(instance, "_toString", em_method_new(instance, to_string));

	*value = instance;
}

/* initialize emerald */
EM_API em_result_t em_init(em_init_flag_t flags) {

	init_flags = flags;
	if (flags & EM_INIT_FLAG_PRINT_ALLOC_TRAFFIC)
		em_print_allocation_traffic = EM_TRUE;

	if (em_reflist_init(&em_reflist_token) != EM_RESULT_SUCCESS)
		return EM_RESULT_FAILURE;
	if (em_reflist_init(&em_reflist_node) != EM_RESULT_SUCCESS)
		return EM_RESULT_FAILURE;
	if (em_reflist_init(&em_reflist_object) != EM_RESULT_SUCCESS)
		return EM_RESULT_FAILURE;

	em_none = em_none_new();
	em_value_incref(em_none);

	/* create error classes */
	em_class_error = create_error_class("Error", EM_VALUE_FAIL);
	em_class_syntax_error = create_error_class("SyntaxError", em_class_error);
	em_class_runtime_error = create_error_class("RuntimeError", em_class_error);
	em_class_system_break = create_error_class("SystemBreak", EM_VALUE_FAIL);
	em_class_system_continue = create_error_class("SystemContinue", EM_VALUE_FAIL);
	em_class_system_return = create_error_class("SystemReturn", EM_VALUE_FAIL);
	em_class_system_exit = create_error_class("SystemExit", EM_VALUE_FAIL);

	return EM_RESULT_SUCCESS;
}

/* quit emerald */
EM_API void em_quit(void) {

	em_value_decref(em_class_runtime_error);
	em_value_decref(em_class_syntax_error);
	em_value_decref(em_class_error);
	em_value_decref(em_none);
	if (!(init_flags & EM_INIT_FLAG_NO_EXIT_FREE))
		em_reflist_destroy(&em_reflist_object);
	em_reflist_destroy(&em_reflist_node);
	em_reflist_destroy(&em_reflist_token);
	if (!(init_flags & EM_INIT_FLAG_NO_PRINT_ALLOCS))
		em_print_allocs();
}
