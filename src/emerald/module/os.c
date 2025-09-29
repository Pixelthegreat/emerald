/*
 * Copyright 2025, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <emerald/core.h>
#include <emerald/none.h>
#include <emerald/util.h>
#include <emerald/context.h>
#include <emerald/module/os.h>

#if defined _WIN32 || _WIN64
#include <windows.h>
#elif defined _ECLAIR
#include <ec.h>
#else
#include <time.h>
#endif

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

/* sleep for a given amount of time */
static em_value_t os_sleep(em_context_t *context, em_value_t *args, size_t nargs, em_pos_t *pos) {

	em_value_t value;

	if (em_util_parse_args(pos, args, nargs, "n", &value) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

#if defined _WIN32 || _WIN64

#elif defined _ECLAIR

#else
	long ns = 0;
	if (value.type == EM_VALUE_TYPE_INT)
		ns = (long)value.value.te_inttype * 1000000000;
	else ns = (long)(value.value.te_floattype * 1000000000);

	struct timespec ts = {
		.tv_sec = (time_t)(ns / 1000000000),
		.tv_nsec = ns % 1000000000,
	};
	nanosleep(&ts, NULL);
#endif
	return em_none;
}

/* initialize module */
static em_result_t initialize(em_context_t *context, em_value_t map) {

	em_value_t mod = em_map_new();
	em_util_set_value(map, "__module_os", mod);

	/* system info */
	em_value_t sysinfo = em_map_new();
	em_util_set_string(sysinfo, "name", OS_NAME);

	em_util_set_value(mod, "info", sysinfo);

	/* functions */
	em_util_set_function(mod, "sleep", os_sleep);

	return EM_RESULT_SUCCESS;
}
