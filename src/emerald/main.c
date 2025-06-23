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
#include <emerald/main.h>

em_reflist_t em_reflist_token = EM_REFLIST_INIT;
em_reflist_t em_reflist_node = EM_REFLIST_INIT;

/* initialize emerald */
EM_API em_result_t em_init(void) {

	if (em_reflist_init(&em_reflist_token) != EM_RESULT_SUCCESS)
		return EM_RESULT_FAILURE;
	if (em_reflist_init(&em_reflist_node) != EM_RESULT_SUCCESS)
		return EM_RESULT_FAILURE;
	return EM_RESULT_SUCCESS;
}

/* quit emerald */
EM_API void em_quit(void) {

	em_reflist_destroy(&em_reflist_node);
	em_reflist_destroy(&em_reflist_token);
	em_print_allocs();
}
