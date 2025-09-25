/*
 * Copyright 2025, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <emerald/core.h>
#include <emerald/none.h>
#include <emerald/util.h>
#include <emerald/module/os.h>

/* os module */
static em_result_t initialize(em_context_t *context, em_value_t map);

em_module_t em_module_os = {
	.initialize = initialize,
};

#if defined _WIN32 || _WIN64
#define OS_NAME "windows"
#elif defined _ECLAIR
#define OS_NAME "eclair-os"
#else
#define OS_NAME "posix"
#endif

/* initialize module */
static em_result_t initialize(em_context_t *context, em_value_t map) {

	em_value_t mod = em_map_new();
	em_util_set_value(map, "__module_os", mod);

	/* system info */
	em_value_t sysinfo = em_map_new();
	em_util_set_string(sysinfo, "name", OS_NAME);

	em_util_set_value(mod, "info", sysinfo);

	return EM_RESULT_SUCCESS;
}
