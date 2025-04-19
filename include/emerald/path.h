/*
 * Utilities for manipulating paths
 *
 * The Emerald path format is very much just like Unix
 * style paths. The details are as follows:
 *   - Paths are separated with the '/' delimeter;
 *     Multiple consecutive delimeter characters are
 *     interpreted as one
 *   - Paths that start with one or more of the
 *     delimeter are relative to the root directory
 *   - Paths that end with one or more of the delimeter
 *     are interpreted as if said character was not
 *     present
 *   - Paths can have multibyte characters, so long as
 *     they are encoded with UTF-8
 *   - '.' and '..' represent the current directory
 *     relative to the preceding portion of the path,
 *     and the parent directory, respectively
 *
 * If the OS is not windows, it is assumed that the OS
 * uses Unix style paths under the hood; although this
 * is not a perfect catchall, it should work for all of
 * the major operating systems.
 *
 * Also note that some functions are provided for
 * Emerald's own uint24_t string format, but they
 * otherwise work in the same way.
 */
#ifndef EMERALD_PATH_H
#define EMERALD_PATH_H

#include <emerald/core.h>

#define EM_PATH_DELIM_CHAR '/'

#ifdef EM_WINDOWS
 #define EM_OS_PATH_DELIM_CHAR '\\'
 #define EM_OS_PATH_ROOT_PREFIX "C:\\"
#else
 #define EM_OS_PATH_DELIM_CHAR '/'
 #define EM_OS_PATH_ROOT_PREFIX "/"
#endif

/* functions */
EM_API em_bool_t em_path_exists(const char *path); /* check if file exists */

/* utf-8 functions */
EM_API em_result_t em_path_join(char *buf, size_t cnt, int npaths, const char *start, ...); /* join path names together */
EM_API em_result_t em_path_dirname(char *buf, size_t cnt, const char *path); /* get directory prefix from path */
EM_API em_result_t em_path_basename(char *buf, size_t cnt, const char *path); /* get file name without directory prefix */
EM_API em_result_t em_path_fix(char *buf, size_t cnt, const char *path); /* convert path to operating system format */

/* wide char functions */
EM_API em_result_t em_wpath_join(em_wchar_t *buf, size_t cnt, int npaths, const em_wchar_t *start, ...); /* join wide path names together */
EM_API em_result_t em_wpath_dirname(em_wchar_t *buf, size_t cnt, const em_wchar_t *path); /* get directory prefix from wide path */
EM_API em_result_t em_wpath_basename(em_wchar_t *buf, size_t cnt, const em_wchar_t *path); /* get file name without directory prefix */
EM_API em_result_t em_wpath_fix(char *buf, size_t cnt, const em_wchar_t *path); /* convert wide path to operating system format */

#endif /* EMERALD_PATH_H */
