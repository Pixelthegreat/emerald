/*
 * Copyright 2026, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef EMERALD_HASH_H
#define EMERALD_HASH_H

#include <emerald/core.h>
#include <emerald/wchar.h>

/* functions */
EM_API em_hash_t em_utf8_strhash(const char *str); /* generate a unique hash value for a string */
EM_API em_hash_t em_wchar_strhash(const em_wchar_t *str); /* generate a unique hash value for a wide string */

#endif /* EMERALD_HASH_H */
