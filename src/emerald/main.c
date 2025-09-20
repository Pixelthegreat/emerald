/*
 * Copyright 2025, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <emerald/core.h>
#include <emerald/memory.h>
#include <emerald/refobj.h>
#include <emerald/none.h>
#include <emerald/main.h>

em_reflist_t em_reflist_token = EM_REFLIST_INIT;
em_reflist_t em_reflist_node = EM_REFLIST_INIT;
em_reflist_t em_reflist_object = EM_REFLIST_INIT;
em_value_t em_none = EM_VALUE_FAIL;

static em_init_flag_t init_flags;

/* initialize emerald */
EM_API em_result_t em_init(em_init_flag_t flags) {

	init_flags = flags;

	if (em_reflist_init(&em_reflist_token) != EM_RESULT_SUCCESS)
		return EM_RESULT_FAILURE;
	if (em_reflist_init(&em_reflist_node) != EM_RESULT_SUCCESS)
		return EM_RESULT_FAILURE;
	if (em_reflist_init(&em_reflist_object) != EM_RESULT_SUCCESS)
		return EM_RESULT_FAILURE;

	em_none = em_none_new();
	em_value_incref(em_none);

	return EM_RESULT_SUCCESS;
}

/* quit emerald */
EM_API void em_quit(void) {

	em_value_decref(em_none);
	if (!(init_flags & EM_INIT_FLAG_NO_EXIT_FREE))
		em_reflist_destroy(&em_reflist_object);
	em_reflist_destroy(&em_reflist_node);
	em_reflist_destroy(&em_reflist_token);
	if (!(init_flags & EM_INIT_FLAG_NO_PRINT_ALLOCS))
		em_print_allocs();
}
