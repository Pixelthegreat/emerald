/*
 * Copyright 2025, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef EMERALD_CONTEXT_H
#define EMERALD_CONTEXT_H

#include <emerald/core.h>
#include <emerald/lexer.h>
#include <emerald/parser.h>

/* context */
#define EM_CONTEXT_MAX_DIRS 32

typedef struct em_recfile {
	struct em_recfile *next; /* next entry */
	char rpath[]; /* real file path */
} em_recfile_t;

typedef struct em_context {
	em_bool_t init; /* initialized */
	em_lexer_t lexer; /* local lexer */
	em_parser_t parser; /* local parser */
	const char *dirstack[EM_CONTEXT_MAX_DIRS]; /* directory stack */
	size_t ndirstack; /* number of directories in stack */
	em_recfile_t *rec_first; /* first run file */
	em_recfile_t *rec_last; /* last run file */
} em_context_t;

#define EM_CONTEXT_INIT ((em_context_t){EM_FALSE})

/* functions */
extern em_context_t *em_context_new(void); /* create context */
extern em_result_t em_context_init(em_context_t *context); /* initialize context */
extern em_result_t em_context_run_text(em_context_t *context, const char *path, const char *text, em_ssize_t len); /* run code */
extern const char *em_context_pushdir(em_context_t *context, const char *path); /* push directory to stack */
extern const char *em_context_resolve(em_context_t *context, const char *path); /* resolve file path */
extern const char *em_context_popdir(em_context_t *context); /* pop directory from stack */
extern em_result_t em_context_run_file(em_context_t *context, em_pos_t *pos, const char *path); /* run code from file */
extern void em_context_destroy(em_context_t *context); /* destroy context */
extern void em_context_free(em_context_t *context); /* free context */

#endif /* EMERALD_CONTEXT_H */
