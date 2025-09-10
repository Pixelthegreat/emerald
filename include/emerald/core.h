/*
 * Copyright 2025, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef EM_CORE_H
#define EM_CORE_H

#include <stddef.h>
#include <stdint.h>
#include <limits.h>

/* system macros */
#if defined _WIN32 || defined _WIN64
 #ifdef EM_LIB
  #define EM_API extern __declspec(dllexport)
 #else
  #define EM_API extern __declspec(dllimport)
 #endif
 #define EM_WINDOWS
#else
 #define EM_API extern
#endif

#define EM_STDLIB_DIR "stdlib"

#define EM_INLINE static inline

/* result */
typedef enum em_result {
	EM_RESULT_SUCCESS = 0,
	EM_RESULT_FAILURE,

	EM_RESULT_COUNT,
} em_result_t;

/* helper macros */
#define EM_MIN(a, b) ((a) < (b)? (a): (b))
#define EM_MAX(a, b) ((a) > (b)? (a): (b))
#define EM_CLAMP(x, a, b) EM_MIN(EM_MAX((x), (a)), (b))

/* random/miscellaneous types */
typedef uint8_t em_bool_t;

#define EM_TRUE ((em_bool_t)1)
#define EM_FALSE ((em_bool_t)0)

typedef long em_ssize_t;
#define EM_SSIZE_MAX LONG_MAX

typedef uint32_t em_hash_t;
#define EM_HASH_MAX UINT32_MAX
#define EM_HASH_BITS 32

/* a type for use by integers within the scope of the language */
typedef long em_inttype_t;
#define EM_INTTYPE_MAX LONG_MAX

#define EM_INTTYPE_FORMAT "%ld"

typedef double em_floattype_t;

#define EM_FLOATTYPE_FORMAT "%lg"
#define EM_FLOATTYPE_MOD fmod

/* an absurd concept: uint24_t */
typedef struct em_uint24 {
	uint8_t a, b, c;
} __attribute__((packed)) em_uint24_t;

#define EM_U24(v) ((em_uint24_t){.a = (uint8_t)(v & 0xff), .b = (uint8_t)((v >> 8) & 0xff), .c = (uint8_t)((v >> 16) & 0xff)})
#define EM_INT(v) (((int)((v).c) << 16) | ((int)((v).b) << 8) | (int)((v).a))

#define EM_U24EQ(x, y) (((x).a == (y).a) && ((x).b == (y).b) && ((x).c == (y).c))

#define EM_U24Z(x) (((x).a == 0) && ((x).b == 0) && ((x).c == 0))

typedef em_uint24_t em_wchar_t; /* useful for wide but not completely wide characters */

#define EM_INT2WC EM_U24
#define EM_WC2INT EM_INT
#define EM_WCEQ EM_U24EQ
#define EM_WCZ EM_U24Z

struct em_context;

#endif /* EM_CORE_H */
