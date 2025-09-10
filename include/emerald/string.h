/*
 * Copyright 2025, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef EMERALD_STRING_H
#define EMERALD_STRING_H

#include <emerald/core.h>
#include <emerald/object.h>

/* string */
typedef struct em_string {
	em_object_t base;
	size_t length; /* string length */
	em_hash_t hash; /* string hash value */
	em_wchar_t data[]; /* string data */
} em_string_t;

#define EM_STRING(p) ((em_string_t *)(p))

/* functions */
EM_API em_value_t em_string_new(size_t length); /* create string */
EM_API em_value_t em_string_new_from_utf8(const char *data, size_t length); /* create string from utf8 data */
EM_API em_value_t em_string_new_from_wchar(const em_wchar_t *data, size_t length); /* create string from wchar data */
EM_API em_bool_t em_is_string(em_value_t v); /* determine if value is a string */

#endif /* EMERALD_STRING_H */
