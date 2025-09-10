/*
 * Copyright 2025, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef EMERALD_MODULE_H
#define EMERALD_MODULE_H

#include <emerald/core.h>
#include <emerald/object.h>
#include <emerald/context.h>

/* module */
typedef struct em_module {
	em_result_t (*initialize)(em_context_t *); /* initialize module */
} em_module_t;

/* functions */
EM_API em_result_t em_module_init_all(em_context_t *context); /* initialize all modules */

#endif /* EMERALD_MODULE_H */
