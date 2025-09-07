/*
 * Copyright 2025, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Dynamic array implementation
 */
#ifndef EMERALD_ARRAY_H
#define EMERALD_ARRAY_H

#include <emerald/core.h>

/* generic value types */
/*  (t|te)_[c,u]type[p]
 *   t  - libc type
 *   te - emerald type
 *   c  - const
 *   u  - unsigned
 *   p  - pointer
 */
typedef union em_generic {
	char         t_char;
	short        t_short;
	int          t_int;
	long         t_long;
	long long    t_longlong;
	int8_t       t_int8;
	uint8_t      t_uint8;
	int16_t      t_int16;
	uint16_t     t_uint16;
	int32_t      t_int32;
	uint32_t     t_uint32;
	int64_t      t_int64;
	uint64_t     t_uint64;
	size_t       t_size;
	void        *t_voidp;
	const void  *t_cvoidp;
	char        *t_charp;
	const char  *t_ccharp;
	float        t_float;
	double       t_double;

	/* emerald types */
	em_bool_t    te_bool;
	em_ssize_t   te_ssize;
	em_hash_t    te_hash;
	em_inttype_t te_inttype;
	em_uint24_t  te_uint24;
	em_wchar_t   te_wchar;
	em_floattype_t te_floattype;
} em_generic_t;

typedef struct em_generic_result {
	em_generic_t v; /* result value */
	em_bool_t p; /* value present */
} em_generic_result_t;

#define EM_GENERIC_NONE ((em_generic_result_t){.p = EM_FALSE})

/* array */
#define EM_ARRAY_BASE_ITEMS 8

typedef struct em_array {
	em_bool_t init; /* initialized */
	em_generic_t base[EM_ARRAY_BASE_ITEMS]; /* base items */
	em_generic_t *ext; /* extended array */
	size_t nitems; /* number of items */
	size_t cext; /* capacity of extended array */
} em_array_t;

#define EM_ARRAY_INIT ((em_array_t){EM_FALSE})

/* functions */
EM_API em_result_t em_array_init(em_array_t *array); /* initialize array */
EM_API em_result_t em_array_add(em_array_t *array, em_generic_t value); /* add value */
EM_API em_generic_result_t em_array_get(em_array_t *array, size_t index); /* get array value */
EM_API void em_array_destroy(em_array_t *array); /* destroy array */

#endif /* EMERALD_ARRAY_H */
