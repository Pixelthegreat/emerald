/*
 * Copyright 2025, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <emerald/core.h>
#include <emerald/module.h>

#include "modules.h"

/* initialize all modules */
EM_API em_result_t em_module_init_all(em_context_t *context) {

	size_t nmodules = sizeof(modules) / sizeof(modules[0]);
	for (size_t i = 0; i < nmodules; i++) {

		if (modules[i]->initialize && modules[i]->initialize(context, context->scopestack[0]) != EM_RESULT_SUCCESS)
			return EM_RESULT_FAILURE;
	}
	return EM_RESULT_SUCCESS;
}

/* destroy all modules */
EM_API void em_module_destroy_all(em_context_t *context) {

	if (!context) return;

	size_t nmodules = sizeof(modules) / sizeof(modules[0]);
	for (size_t i = 0; i < nmodules; i++) {

		if (modules[i]->destroy)
			modules[i]->destroy(context);
	}
}
