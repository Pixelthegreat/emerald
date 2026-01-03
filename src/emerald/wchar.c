/*
 * Copyright 2026, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <emerald/core.h>
#include <emerald/utf8.h>
#include <emerald/wchar.h>

/* search wide string from beginning for character */
EM_API em_wchar_t *em_wchar_strchr(const em_wchar_t *str, em_wchar_t ch) {

	while (!EM_WCZ(*str)) {

		if (EM_WCEQ(*str, ch)) return (em_wchar_t *)str;
		str++;
	}
	return NULL;
}

/* search wide string from end for character */
EM_API em_wchar_t *em_wchar_strrchr(const em_wchar_t *str, em_wchar_t ch) {

	em_wchar_t *stop = NULL;
	while (!EM_WCZ(*str)) {

		if (EM_WCEQ(*str, ch)) stop = (em_wchar_t *)str;
		str++;
	}
	return stop;
}

/* get length of wide string */
EM_API size_t em_wchar_strlen(const em_wchar_t *str) {

	size_t n = 0;
	while (!EM_WCZ(*str)) { n++; str++; }
	return n;
}

/* copy utf-8 string to wide string */
EM_API em_result_t em_wchar_from_utf8(em_wchar_t *buf, size_t cnt, const char *src) {

	size_t i = 0;
	while (*src && i < cnt-1) {

		em_ssize_t nbytes;
		int ch = em_utf8_getch(src, &nbytes);
		if (ch < 0) return EM_RESULT_FAILURE;

		buf[i] = EM_INT2WC(ch);
		src += nbytes;
		i++;
	}
	buf[i] = EM_INT2WC(0);
	return EM_RESULT_SUCCESS;
}

/* copy wide string to utf-8 string */
EM_API em_result_t em_wchar_to_utf8(char *buf, size_t cnt, const em_wchar_t *src) {

	size_t i = 0;
	while (!EM_WCZ(*src) && i < cnt-4) {

		int ch = EM_WC2INT(*src);

		em_ssize_t nbytes = em_utf8_putch(buf+i, ch);
		if (nbytes < 1 || nbytes > 4)
			return EM_RESULT_FAILURE;
		i += (size_t)nbytes;
		src++;
	}
	buf[i] = 0;
	return EM_RESULT_SUCCESS;
}

/* write wide string to file */
EM_API em_result_t em_wchar_write(FILE *fp, const em_wchar_t *str, size_t cnt) {

	for (size_t i = 0; i < cnt; i++) {

		char buf[5];
		if (em_wchar_to_utf8(buf, 5, str+i) != EM_RESULT_SUCCESS)
			return EM_RESULT_FAILURE;

		size_t len = strlen(buf);
		if (fwrite(buf, 1, len, fp) != len)
			return EM_RESULT_FAILURE;
	}
	return EM_RESULT_SUCCESS;
}
