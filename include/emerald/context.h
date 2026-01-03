/*
 * Copyright 2026, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef EMERALD_CONTEXT_H
#define EMERALD_CONTEXT_H

#include <emerald/core.h>
#include <emerald/lexer.h>
#include <emerald/parser.h>
#include <emerald/value.h>

/* context */
#define EM_CONTEXT_MAX_DIRS 32
#define EM_CONTEXT_MAX_SCOPE 128

typedef struct em_recfile {
	struct em_recfile *next; /* next entry */
	char rpath[]; /* real file path */
} em_recfile_t;

typedef struct em_context {
	em_bool_t init; /* initialized */
	const char **argv; /* cli arguments */
	em_lexer_t lexer; /* local lexer */
	em_parser_t parser; /* local parser */
	const char *dirstack[EM_CONTEXT_MAX_DIRS]; /* directory stack */
	size_t ndirstack; /* number of directories in stack */
	em_value_t scopestack[EM_CONTEXT_MAX_SCOPE]; /* scope stack */
	size_t nscopestack; /* number of scopes in stack */
	em_recfile_t *rec_first; /* first run file */
	em_recfile_t *rec_last; /* last run file */
	em_value_t pass; /* value to pass down for return statement */
} em_context_t;

#define EM_CONTEXT_INIT ((em_context_t){EM_FALSE})

/* functions */
EM_API em_context_t *em_context_new(const char **argv); /* create context */
EM_API em_result_t em_context_init(em_context_t *context, const char **argv); /* initialize context */
EM_API em_value_t em_context_run_text(em_context_t *context, const char *path, const char *text, em_ssize_t len); /* run code */
EM_API const char *em_context_pushdir(em_context_t *context, const char *path); /* push directory to stack */
EM_API const char *em_context_resolve(em_context_t *context, const char *path); /* resolve file path */
EM_API const char *em_context_popdir(em_context_t *context); /* pop directory from stack */
EM_API em_result_t em_context_push_scope(em_context_t *context); /* push scope to stack */
EM_API void em_context_pop_scope(em_context_t *context); /* pop scope from stack */
EM_API void em_context_set_value(em_context_t *context, em_hash_t key, em_value_t value); /* set value in current scope */
EM_API em_value_t em_context_get_value(em_context_t *context, em_hash_t key); /* get value from current scope */
EM_API em_value_t em_context_run_file(em_context_t *context, em_pos_t *pos, const char *path); /* run code from file */

EM_API em_value_t em_context_visit(em_context_t *context, em_node_t *node); /* visit node */
EM_API em_value_t em_context_visit_block(em_context_t *context, em_node_t *node); /* visit block */
EM_API em_value_t em_context_visit_int(em_context_t *context, em_node_t *node); /* visit integer */
EM_API em_value_t em_context_visit_float(em_context_t *context, em_node_t *node); /* visit float */
EM_API em_value_t em_context_visit_string(em_context_t *context, em_node_t *node); /* visit string */
EM_API em_value_t em_context_visit_identifier(em_context_t *context, em_node_t *node); /* visit identifier */
EM_API em_value_t em_context_visit_list(em_context_t *context, em_node_t *node); /* visit list */
EM_API em_value_t em_context_visit_map(em_context_t *context, em_node_t *node); /* visit map */
EM_API em_value_t em_context_visit_unary_operation(em_context_t *context, em_node_t *node); /* visit unary operation */
EM_API em_value_t em_context_visit_binary_operation(em_context_t *context, em_node_t *node); /* visit binary operation */
EM_API em_value_t em_context_visit_access(em_context_t *context, em_node_t *node); /* visit member access */
EM_API em_value_t em_context_visit_call(em_context_t *context, em_node_t *node); /* visit call */
EM_API em_value_t em_context_visit_continue(em_context_t *context, em_node_t *node); /* visit continue statement */
EM_API em_value_t em_context_visit_break(em_context_t *context, em_node_t *node); /* visit break statement */
EM_API em_value_t em_context_visit_return(em_context_t *context, em_node_t *node); /* visit return statement */
EM_API em_value_t em_context_visit_raise(em_context_t *context, em_node_t *node); /* visit raise statement */
EM_API em_value_t em_context_visit_include(em_context_t *context, em_node_t *node); /* visit include statement */
EM_API em_value_t em_context_visit_let(em_context_t *context, em_node_t *node); /* visit let statement */
EM_API em_value_t em_context_visit_if(em_context_t *context, em_node_t *node); /* visit if statement */
EM_API em_value_t em_context_visit_for(em_context_t *context, em_node_t *node); /* visit for statement */
EM_API em_value_t em_context_visit_foreach(em_context_t *context, em_node_t *node); /* visit foreach statement */
EM_API em_value_t em_context_visit_while(em_context_t *context, em_node_t *node); /* visit while statement */
EM_API em_value_t em_context_visit_func(em_context_t *context, em_node_t *node); /* visit func statement */
EM_API em_value_t em_context_visit_class(em_context_t *context, em_node_t *node); /* visit class statement */
EM_API em_value_t em_context_visit_try(em_context_t *context, em_node_t *node); /* visit try statement */
EM_API em_value_t em_context_visit_puts(em_context_t *context, em_node_t *node); /* visit puts statement */

EM_API void em_context_destroy(em_context_t *context); /* destroy context */
EM_API void em_context_free(em_context_t *context); /* free context */

#endif /* EMERALD_CONTEXT_H */
