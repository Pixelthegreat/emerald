/*
 * Copyright 2025, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <emerald/core.h>
#include <emerald/utf8.h>
#include <emerald/hash.h>

/* raise to a power */
static int hashtpow(em_hash_t x, em_hash_t y) {

	em_hash_t r = 1;
	for (em_hash_t i = 0; i < y; i++)
		r *= x;
	return r;
}

/* generate a unique hash value for a string */
EM_API em_hash_t em_utf8_strhash(const char *str) {

	if (!str[0]) return 0;

	em_hash_t res = 0;
	em_hash_t len = (em_hash_t)em_utf8_strlen(str);

	int ch; em_ssize_t nbytes;
	for (em_hash_t i = 0; i < len-1; i++) {

		ch = em_utf8_getch(str, &nbytes);
		res += (em_hash_t)ch * hashtpow(EM_HASH_BITS-1, len-i-1);
		if (nbytes >= 1 && nbytes <= 4) str += nbytes;
	}
	ch = em_utf8_getch(str, &nbytes);
	res += (em_hash_t)ch;

	return res;
}

/* generate a unique hash value for a wide string */
EM_API em_hash_t em_wchar_strhash(const em_wchar_t *str) {

	if (EM_WCZ(str[0])) return 0;

	em_hash_t res = 0;
	em_hash_t len = (em_hash_t)em_wchar_strlen(str);

	int ch;
	for (em_hash_t i = 0; i < len-1; i++) {

		ch = EM_WC2INT(str[i]);
		res += (em_hash_t)ch * hashtpow(EM_HASH_BITS-1, len-i-1);
	}
	ch = EM_WC2INT(str[len-1]);
	res += (em_hash_t)ch;

	return res;
}
