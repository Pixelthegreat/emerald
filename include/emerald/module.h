/*
 * Copyright 2026, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef EMERALD_MODULE_H
#define EMERALD_MODULE_H

#include <emerald/core.h>
#include <emerald/object.h>
#include <emerald/context.h>
#include <emerald/map.h>

/* module */
typedef struct em_module {
	em_result_t (*initialize)(em_context_t *, em_value_t); /* initialize module */
	void (*destroy)(em_context_t *); /* clean up module resources */
} em_module_t;

/* functions */
EM_API em_result_t em_module_init_all(em_context_t *context); /* initialize all modules */
EM_API void em_module_destroy_all(em_context_t *context); /* destroy all modules */

#endif /* EMERALD_MODULE_H */
