/*
 * Copyright 2026, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef EMERALD_MAIN_H
#define EMERALD_MAIN_H

#include <emerald/core.h>

/* initialize flags */
typedef enum em_init_flag {
	EM_INIT_FLAG_NO_EXIT_FREE = 0x1,
	EM_INIT_FLAG_NO_PRINT_ALLOCS = 0x2,
	EM_INIT_FLAG_PRINT_ALLOC_TRAFFIC = 0x4,
} em_init_flag_t;

/* functions */
EM_API em_result_t em_init(em_init_flag_t flags); /* initialize emerald */
EM_API void em_quit(void); /* quit emerald */

#endif /* EMERALD_MAIN_H */
