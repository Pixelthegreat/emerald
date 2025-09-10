/*
 * Copyright 2025, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef EMERALD_NONE_H
#define EMERALD_NONE_H

#include <emerald/core.h>
#include <emerald/object.h>

EM_API em_value_t em_none;

/* functions */
EM_API em_value_t em_none_new(void); /* create none */

#endif /* EMERALD_NONE_H */
