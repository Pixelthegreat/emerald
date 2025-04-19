#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <emerald/core.h>
#include <emerald/utf8.h>
#include <emerald/wchar.h>
#include <emerald/path.h>

#ifdef EM_WINDOWS
#include <windows.h>
#else
#include <sys/stat.h>
#endif

/* check if file exists */
EM_API em_bool_t em_path_exists(const char *path) {

#ifdef EM_WINDOWS
	DWORD attributes = GetFileAttributesA(path);
	return (em_bool_t)(attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY));
#else
	struct stat st;
	if (stat(path, &st) < 0) return EM_FALSE;
	
	return (em_bool_t)S_ISREG(st.st_mode);
#endif
}

/* join path names together */
EM_API em_result_t em_path_join(char *buf, size_t cnt, int npaths, const char *start, ...) {

	va_list args;
	va_start(args, start);

	size_t nbytes = 0; /* number of bytes copied */
	em_bool_t delim = (start[0] == EM_PATH_DELIM_CHAR)? EM_TRUE: EM_FALSE; /* include delimeter in next copy */
	for (int i = 0; i < npaths; i++) {

		const char *tok = i? va_arg(args, const char *): start;
		while (tok) {

			const char *end = em_utf8_strchr(tok, EM_PATH_DELIM_CHAR);
			const char *next = end? end+1: NULL;
			size_t len = end? (size_t)(end - tok): strlen(tok);

			/* skip section */
			if (!len) {
				tok = next;
				continue;
			}

			/* copy string */
			em_ssize_t chlen = em_utf8_getchlen(EM_PATH_DELIM_CHAR);
			if (chlen < 1 || chlen > 4) return EM_RESULT_FAILURE;

			if (nbytes+chlen+len > cnt-1) {
				
				va_end(args);
				return EM_RESULT_FAILURE;
			}

			if (delim && em_utf8_putch(buf+nbytes, EM_PATH_DELIM_CHAR) != chlen) {

				va_end(args);
				return EM_RESULT_FAILURE;
			}
			else if (delim) nbytes += (size_t)chlen;

			delim = EM_TRUE; /* include delimiter in next copy */
			memcpy(buf+nbytes, tok, len);
			nbytes += len;

			tok = next;
		}
	}
	buf[nbytes] = 0;

	va_end(args);
	return EM_RESULT_SUCCESS;
}

/* get directory prefix from path */
EM_API em_result_t em_path_dirname(char *buf, size_t cnt, const char *path) {

	const char *start = path, *end = path+strlen(path);
	
	/* get start position */
	em_ssize_t nbytes;
	int ch = em_utf8_getch(start, &nbytes);
	while (ch == EM_PATH_DELIM_CHAR) {

		start += nbytes;
		ch = em_utf8_getch(start, &nbytes);
	}
	if (ch < 0) return EM_RESULT_FAILURE;

	if (start > path) start -= nbytes;

	/* get end position */
	ch = em_utf8_rgetch(start, end, &nbytes);
	while (ch == EM_PATH_DELIM_CHAR) {

		end -= nbytes;
		ch = em_utf8_rgetch(start, end, &nbytes);
	}
	if (ch < 0) return EM_RESULT_FAILURE;

	/* move end position past file name */
	ch = em_utf8_rgetch(start, end, &nbytes);
	while (end > start && ch != EM_PATH_DELIM_CHAR) {

		end -= nbytes;
		ch = em_utf8_rgetch(start, end, &nbytes);
	}
	if (ch < 0) return EM_RESULT_FAILURE;

	/* move past delimeter */
	while (end > start && ch == EM_PATH_DELIM_CHAR) {

		end -= nbytes;
		ch = em_utf8_rgetch(start, end, &nbytes);
	}

	/* copy name */
	size_t len = (size_t)(end - start);
	memcpy(buf, start, len);
	buf[len] = 0;

	return EM_RESULT_SUCCESS;
}

/* get file name without directory prefix */
EM_API em_result_t em_path_basename(char *buf, size_t cnt, const char *path) {

	const char *start = path, *end = path+strlen(path);
	
	/* get start position */
	em_ssize_t nbytes;
	int ch = em_utf8_getch(start, &nbytes);
	while (ch == EM_PATH_DELIM_CHAR) {

		start += nbytes;
		ch = em_utf8_getch(start, &nbytes);
	}
	if (ch < 0) return EM_RESULT_FAILURE;

	/* get end position */
	ch = em_utf8_rgetch(start, end, &nbytes);
	while (ch == EM_PATH_DELIM_CHAR) {

		end -= nbytes;
		ch = em_utf8_rgetch(start, end, &nbytes);
	}
	if (ch < 0) return EM_RESULT_FAILURE;

	/* move start position past directory name */
	const char *next = start;
	while (next < end) {

		ch = em_utf8_getch(next, &nbytes);
		if (ch < 0) return EM_RESULT_FAILURE;

		next += nbytes;
		if (ch == EM_PATH_DELIM_CHAR) start = next;
	}
	if (start > end) start = end;

	/* copy name */
	size_t len = (size_t)(end - start);
	memcpy(buf, start, len);
	buf[len] = 0;

	return EM_RESULT_SUCCESS;
}

/* convert path to operating system format */
EM_API em_result_t em_path_fix(char *buf, size_t cnt, const char *path) {

	size_t nbytes = 0;
	em_bool_t delim = EM_FALSE;

	/* add root directory */
	if (path[0] == EM_PATH_DELIM_CHAR) {

		const char *prefix = EM_OS_PATH_ROOT_PREFIX;
		size_t len = strlen(prefix);

		if (len > cnt-1) return EM_RESULT_FAILURE;

		memcpy(buf, prefix, len);
		nbytes += len;
	}

	/* add paths */
	const char *tok = path;
	while (tok) {

		const char *end = em_utf8_strchr(tok, EM_PATH_DELIM_CHAR);
		const char *next = end? end+1: NULL;
		size_t len = end? (size_t)(end - tok): strlen(tok);

		/* skip section */
		if (!len) {
			tok = next;
			continue;
		}

		/* copy string */
		em_ssize_t chlen = em_utf8_getchlen(EM_OS_PATH_DELIM_CHAR);
		if (chlen < 1 || chlen > 4) return EM_RESULT_FAILURE;

		if (nbytes+chlen+len > cnt-1) return EM_RESULT_FAILURE;

		if (delim && em_utf8_putch(buf+nbytes, EM_OS_PATH_DELIM_CHAR) != chlen)
			return EM_RESULT_FAILURE;
		else if (delim) nbytes += (size_t)chlen;

		delim = EM_TRUE;
		memcpy(buf+nbytes, tok, len);
		nbytes += len;

		tok = next;
	}

	buf[nbytes] = 0;
	return EM_RESULT_SUCCESS;
}

/* join wide path names together */
EM_API em_result_t em_wpath_join(em_wchar_t *buf, size_t cnt, int npaths, const em_wchar_t *start, ...) {

	va_list args;
	va_start(args, start);

	em_wchar_t delch = EM_INT2WC(EM_PATH_DELIM_CHAR);

	size_t nchars = 0; /* number of characters copied */
	em_bool_t delim = EM_WCEQ(start[0], delch)? EM_TRUE: EM_FALSE; /* include delimeter in next copy */
	for (int i = 0; i < npaths; i++) {

		const em_wchar_t *tok = i? va_arg(args, const em_wchar_t *): start;
		while (tok) {

			const em_wchar_t *end = em_wchar_strchr(tok, delch);
			const em_wchar_t *next = end? end+1: NULL;
			size_t len = end? (size_t)(end - tok): em_wchar_strlen(tok);

			/* skip section */
			if (!len) {
				tok = next;
				continue;
			}

			/* copy string */
			if (nchars+1+len > cnt-1) {
				
				va_end(args);
				return EM_RESULT_FAILURE;
			}

			if (delim) buf[nchars++] = delch;

			delim = EM_TRUE; /* include delimiter in next copy */
			memcpy(buf+nchars, tok, len * sizeof(em_wchar_t));
			nchars += len;

			tok = next;
		}
	}
	buf[nchars] = EM_INT2WC(0);

	va_end(args);
	return EM_RESULT_SUCCESS;
}

/* get directory prefix from wide path */
EM_API em_result_t em_wpath_dirname(em_wchar_t *buf, size_t cnt, const em_wchar_t *path) {

	const em_wchar_t *start = path, *end = path + em_wchar_strlen(path);

	em_wchar_t delch = EM_INT2WC(EM_PATH_DELIM_CHAR);
	
	/* get start and end positions */
	while (EM_WCEQ(*start, delch)) start++;
	if (start > path) start--;

	while (end > start && EM_WCEQ(*(end-1), delch)) end--;

	/* move end position past file name */
	while (end > start && !EM_WCEQ(*(end-1), delch)) end--;

	while (end > start && EM_WCEQ(*(end-1), delch)) end--;

	/* copy name */
	size_t len = (size_t)(end - start);
	memcpy(buf, start, len * sizeof(em_wchar_t));
	buf[len] = EM_INT2WC(0);

	return EM_RESULT_SUCCESS;
}

/* get file name without directory prefix */
EM_API em_result_t em_wpath_basename(em_wchar_t *buf, size_t cnt, const em_wchar_t *path) {

	const em_wchar_t *start = path, *end = path + em_wchar_strlen(path);

	em_wchar_t delch = EM_INT2WC(EM_PATH_DELIM_CHAR);
	
	/* get start and end positions */
	while (EM_WCEQ(*start, delch)) start++;

	while (end > start && EM_WCEQ(*(end-1), delch)) end--;

	/* move start position past directory name */
	const em_wchar_t *next = start;
	while (next < end) {

		em_wchar_t ch = *next;
		next++;
		if (EM_WCEQ(ch, delch)) start = next;
	}
	if (start > end) start = end;

	/* copy name */
	size_t len = (size_t)(end - start);
	memcpy(buf, start, len * sizeof(em_wchar_t));
	buf[len] = EM_INT2WC(0);

	return EM_RESULT_SUCCESS;
}

/* convert wide path to operating system format */
EM_API em_result_t em_wpath_fix(char *buf, size_t cnt, const em_wchar_t *path) {

	size_t nbytes = 0;
	em_bool_t delim = EM_FALSE;

	em_wchar_t delch = EM_INT2WC(EM_PATH_DELIM_CHAR);
	em_wchar_t osdelch = EM_INT2WC(EM_OS_PATH_DELIM_CHAR);

	/* add root directory */
	if (EM_WCEQ(*path, delch)) {

		const char *prefix = EM_OS_PATH_ROOT_PREFIX;
		size_t len = strlen(prefix);

		if (len > cnt-1) return EM_RESULT_FAILURE;

		memcpy(buf, prefix, len);
		nbytes += len;
	}

	/* add paths */
	const em_wchar_t *tok = path;
	while (tok) {

		const em_wchar_t *end = em_wchar_strchr(tok, delch);
		const em_wchar_t *next = end? end+1: NULL;
		size_t wlen = end? (size_t)(end - tok): em_wchar_strlen(tok);

		/* skip section */
		if (!wlen) {
			tok = next;
			continue;
		}

		/* copy string */
		em_ssize_t chlen = em_utf8_getchlen(EM_OS_PATH_DELIM_CHAR);
		if (chlen < 1 || chlen > 4) return EM_RESULT_FAILURE;

		if (nbytes+chlen > cnt-1) return EM_RESULT_FAILURE;

		if (delim && em_utf8_putch(buf+nbytes, EM_OS_PATH_DELIM_CHAR) != chlen)
			return EM_RESULT_FAILURE;
		else if (delim) nbytes += (size_t)chlen;

		delim = EM_TRUE;
		for (size_t i = 0; i < wlen; i++) {

			int ch = EM_WC2INT(tok[i]);
			chlen = em_utf8_getchlen(ch);
			if (nbytes+chlen > cnt-1) return EM_RESULT_FAILURE;

			if (em_utf8_putch(buf+nbytes, ch) != chlen)
				return EM_RESULT_FAILURE;
			nbytes += (size_t)chlen;
		}

		tok = next;
	}

	buf[nbytes] = 0;
	return EM_RESULT_SUCCESS;
}
