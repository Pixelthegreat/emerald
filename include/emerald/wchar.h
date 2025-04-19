/*
 * Copyright 2025, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Emerald wide string manipulation and analysis
 */
#ifndef EMERALD_WCHAR_H
#define EMERALD_WCHAR_H

#include <stdio.h>
#include <emerald/core.h>

/* functions */
EM_API em_wchar_t *em_wchar_strchr(const em_wchar_t *str, em_wchar_t ch); /* search wide string from beginning for character */
EM_API em_wchar_t *em_wchar_strrchr(const em_wchar_t *str, em_wchar_t ch); /* search wide string from end for character */
EM_API size_t em_wchar_strlen(const em_wchar_t *str); /* get length of wide string */
EM_API em_result_t em_wchar_from_utf8(em_wchar_t *buf, size_t cnt, const char *src); /* copy utf-8 string to wide string */
EM_API em_result_t em_wchar_to_utf8(char *buf, size_t cnt, const em_wchar_t *src); /* copy wide string to utf-8 string */
EM_API em_result_t em_wchar_write(FILE *fp, const em_wchar_t *str, size_t cnt); /* write wide string to file */

#endif /* EMERALD_WCHAR_H */
