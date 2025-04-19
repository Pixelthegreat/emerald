/*
 * Copyright 2025, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <emerald/core.h>
#include <emerald/utf8.h>

#define UTF8_START4 0b11110000
#define UTF8_MASK4  0b11111000

#define UTF8_START3 0b11100000
#define UTF8_MASK3  0b11110000

#define UTF8_START2 0b11000000
#define UTF8_MASK2  0b11100000

#define UTF8_STARTB 0b10000000
#define UTF8_MASKB  0b11000000

/* get character from utf-8 string */
EM_API int em_utf8_getch(const char *src, em_ssize_t *nbytes) {

	*nbytes = -1;
	uint8_t *bsrc = (uint8_t *)src;

	int res = 0;
	uint8_t b0 = *bsrc++;

	/* four byte character */
	if ((b0 & UTF8_MASK4) == UTF8_START4) {

		uint8_t b1 = *bsrc++;
		uint8_t b2 = *bsrc++;
		uint8_t b3 = *bsrc++;

		if (((b1 & UTF8_MASKB) != UTF8_STARTB) ||
		    ((b2 & UTF8_MASKB) != UTF8_STARTB) ||
		    ((b3 & UTF8_MASKB) != UTF8_STARTB))
			return -1;

		res = ((int)b0 & 0x7) << 18;
		res |= ((int)b1 & 0x3f) << 12;
		res |= ((int)b2 & 0x3f) << 6;
		res |= (int)b3 & 0x3f;
		*nbytes = 4;
	}

	/* three byte character */
	else if ((b0 & UTF8_MASK3) == UTF8_START3) {

		uint8_t b1 = *bsrc++;
		uint8_t b2 = *bsrc++;

		if (((b1 & UTF8_MASKB) != UTF8_STARTB) ||
		    ((b2 & UTF8_MASKB) != UTF8_STARTB))
			return -1;

		res = ((int)b0 & 0xf) << 12;
		res |= ((int)b1 & 0x3f) << 6;
		res |= (int)b2 & 0x3f;
		*nbytes = 3;
	}

	/* two byte character */
	else if ((b0 & UTF8_MASK2) == UTF8_START2) {

		uint8_t b1 = *bsrc++;

		if ((b1 & UTF8_MASKB) != UTF8_STARTB)
			return -1;

		res = ((int)b0 & 0x1f) << 6;
		res |= (int)b1 & 0x3f;
		*nbytes = 2;
	}

	/* error */
	else if ((b0 & UTF8_MASKB) == UTF8_STARTB)
		return -1;

	/* one byte character */
	else {

		res = (int)b0 & 0x7f;
		*nbytes = 1;
	}

	return res;
}

/* get character from utf-8 string behind pointer */
EM_API int em_utf8_rgetch(const char *base, const char *src, em_ssize_t *nbytes) {

	*nbytes = 0;
	while (src != base && *nbytes < 4) {

		uint8_t b = *(const uint8_t *)--src;
		(*nbytes)++;

		if ((b & UTF8_MASKB) != UTF8_STARTB)
			break;
	}
	return em_utf8_getch(src, nbytes);
}

/* get length of character in bytes as encoded with utf-8 */
EM_API em_ssize_t em_utf8_getchlen(int ch) {

	if (ch < 128) return 1;
	else if (ch < 2048) return 2;
	else if (ch < 65536) return 3;
	else if (ch < 2097152) return 4;
	else return -1;
}

/* put character into utf-8 string */
EM_API em_ssize_t em_utf8_putch(char *dst, int ch) {

	uint8_t *bdst = (uint8_t *)dst;

	em_ssize_t len = em_utf8_getchlen(ch);
	if (len < 1 || len > 4) return -1;

	/* four byte character */
	if (len == 4) {

		*bdst++ = (uint8_t)(((ch >> 18) & 0x7) | UTF8_START4);
		*bdst++ = (uint8_t)(((ch >> 12) & 0x3f) | UTF8_STARTB);
		*bdst++ = (uint8_t)(((ch >> 6) & 0x3f) | UTF8_STARTB);
		*bdst++ = (uint8_t)((ch & 0x3f) | UTF8_STARTB);
	}

	/* three byte character */
	else if (len == 3) {

		*bdst++ = (uint8_t)(((ch >> 12) & 0xf) | UTF8_START3);
		*bdst++ = (uint8_t)(((ch >> 6) & 0x3f) | UTF8_STARTB);
		*bdst++ = (uint8_t)((ch & 0x3f) | UTF8_STARTB);
	}

	/* two byte character */
	else if (len == 2) {

		*bdst++ = (uint8_t)(((ch >> 6) & 0x1f) | UTF8_START2);
		*bdst++ = (uint8_t)((ch & 0x3f) | UTF8_STARTB);
	}

	/* one byte character */
	else *bdst++ = (uint8_t)(ch & 0x7f);

	return len;
}

/* get length of utf-8 string in characters */
EM_API em_ssize_t em_utf8_strlen(const char *src) {

	uint8_t *bsrc = (uint8_t *)src;

	em_ssize_t size = 0;
	size_t pos = 0;
	
	uint8_t b;
	while ((b = bsrc[pos]) != 0) {

		if ((b & UTF8_MASK4) == UTF8_START4) pos += 4;
		else if ((b & UTF8_MASK3) == UTF8_START3) pos += 3;
		else if ((b & UTF8_MASK2) == UTF8_START2) pos += 2;
		else if ((b & UTF8_MASKB) == UTF8_STARTB) return -1;
		else pos++;
		size++;
	}

	return size;
}

/* search utf-8 string from beginning for character */
EM_API char *em_utf8_strchr(const char *str, int ch) {

	while (*str) {
		
		em_ssize_t nbytes;
		if (em_utf8_getch(str, &nbytes) == ch)
			return (char *)str;

		if (nbytes < 0) return NULL;
		str += nbytes;
	}
	return NULL;
}

/* search utf-8 string from end for character */
EM_API char *em_utf8_strrchr(const char *str, int ch) {

	char *stop = NULL;
	while (*str) {

		em_ssize_t nbytes;
		if (em_utf8_getch(str, &nbytes) == ch)
			stop = (char *)str;

		if (nbytes < 0) return NULL;
		str += nbytes;
	}
	return stop;
}
