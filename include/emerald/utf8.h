/*
 * Copyright 2026, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * UTF-8 string manipulation and analysis
 */
#ifndef EMERALD_UTF8_H
#define EMERALD_UTF8_H

#include <emerald/core.h>

/* functions */
EM_API int em_utf8_getch(const char *src, em_ssize_t *nbytes); /* get character from utf-8 string */
EM_API int em_utf8_rgetch(const char *base, const char *src, em_ssize_t *nbytes); /* get character from utf-8 string behind pointer */
EM_API em_ssize_t em_utf8_getchlen(int ch); /* get length of character in bytes as encoded with utf-8 */
EM_API em_ssize_t em_utf8_putch(char *dst, int ch); /* put character into utf-8 string */
EM_API em_ssize_t em_utf8_strlen(const char *src); /* get length of utf-8 string in characters */
EM_API char *em_utf8_strchr(const char *str, int ch); /* search utf-8 string from beginning for character */
EM_API char *em_utf8_strrchr(const char *str, int ch); /* search utf-8 string from end for character */

#endif /* EMERALD_UTF8_H */
