/*
 * Copyright 2025-2026, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Bytecode support
 */
#ifndef EMERALD_BYTECODE_H
#define EMERALD_BYTECODE_H

#include <stdio.h>
#include <emerald/core.h>
#include <emerald/node.h>
#include <emerald/refobj.h>
#include <emerald/value.h>

struct em_context;

/* code object type */
typedef enum em_code_type {
	EM_CODE_TYPE_TREE = 0,
	EM_CODE_TYPE_BINARY,

	EM_CODE_TYPE_COUNT,
} em_code_type_t;

/* bytecode operations */
typedef enum em_code_op {
	EM_CODE_OP_PCINT = 1, /* push int constant */
	EM_CODE_OP_PCFLT, /* push float constant */
	EM_CODE_OP_PCSTR, /* push string constant */
	EM_CODE_OP_PTRUE, /* push true */
	EM_CODE_OP_PFLSE, /* push false */
	EM_CODE_OP_PNONE, /* push none */
	EM_CODE_OP_POP, /* pop value */

	EM_CODE_OP_CLIST, /* construct list */
	EM_CODE_OP_CMAP, /* construct map */

	EM_CODE_OP_UNEG, /* negate value */
	EM_CODE_OP_UNOT, /* logical not value */
	EM_CODE_OP_UBNOT, /* bitwise not value */
	EM_CODE_OP_UINC, /* increment value */
	EM_CODE_OP_UDEC, /* decrement value */

	EM_CODE_OP_BADD, /* add values */
	EM_CODE_OP_BSUB, /* subtract values */
	EM_CODE_OP_BMUL, /* multiply values */
	EM_CODE_OP_BDIV, /* divide values */
	EM_CODE_OP_BMOD, /* modulo values */
	EM_CODE_OP_BBOR, /* bitwise or values */
	EM_CODE_OP_BBXOR, /* bitwise xor values */
	EM_CODE_OP_BBAND, /* bitwise and values */
	EM_CODE_OP_BBLSH, /* bitwise left shift values */
	EM_CODE_OP_BBRSH, /* bitwise right shift values */
	EM_CODE_OP_BEQ, /* compare equality of values */
	EM_CODE_OP_BNEQ, /* compare notted equality of values */
	EM_CODE_OP_BLT, /* compare ordering of values (less) */
	EM_CODE_OP_BGT, /* compare ordering of values (greater) */

	EM_CODE_OP_LOAD, /* load variable */
	EM_CODE_OP_LDNM, /* load named member */
	EM_CODE_OP_LDIDX, /* load indexed member */

	EM_CODE_OP_STOR, /* store variable */
	EM_CODE_OP_STNM, /* store named member */
	EM_CODE_OP_STIDX, /* store indexed member */

	EM_CODE_OP_JMP, /* jump */
	EM_CODE_OP_JTR, /* jump if true */
	EM_CODE_OP_JNTR, /* jump if not true */
	EM_CODE_OP_JPNTR, /* jump and push none if not true */
	EM_CODE_OP_CALL, /* call value */
	EM_CODE_OP_SAVE1, /* save level-1 context (error) */
	EM_CODE_OP_SAVE3, /* save level-3 context (loop) */
	EM_CODE_OP_RSTR1, /* restore level-1 context (error) */
	EM_CODE_OP_RSTR2, /* restore level-2 context (call) */
	EM_CODE_OP_RSTR3, /* restore level-3 context (loop) */
	EM_CODE_OP_DSCD1, /* discard level-1 context (error) */
	EM_CODE_OP_DSCD3, /* discard level-3 context (loop) */

	EM_CODE_OP_DCLS, /* define class */
	EM_CODE_OP_DFUNC, /* define function */
	EM_CODE_OP_DBGN, /* begin definition */

	EM_CODE_OP_ESETL, /* set line */
	EM_CODE_OP_ESETC, /* set column */
	EM_CODE_OP_PUTS, /* print to output */
	EM_CODE_OP_INCLUDE, /* include file */
	EM_CODE_OP_BLTJXPIPI, /* kind of hard to explain */
	EM_CODE_OP_LEN, /* get length of value (doesn't pop value) */
	EM_CODE_OP_R1EISNTP, /* restore level-1 context if error is not type, or push error */

	EM_CODE_OP_COUNT,
} em_code_op_t;

/* portion of bytecode */
typedef struct em_code_slice {
	void *data; /* start of bytecode data */
	size_t position; /* position in bytecode data */
	size_t length; /* length of bytecode data */
	em_code_op_t mode; /* CALL, RSTR1, RSTR2 or RSTR3 */
} em_code_slice_t;

/* code object */
typedef struct em_code {
	em_refobj_t base;
	em_code_type_t type; /* type of object */
	union {
		em_node_t *tree; /* tree-walker node */
		em_code_slice_t binary; /* bytecode slice */
	};
	char path[]; /* file path */
} em_code_t;

#define EM_CODE(p) ((em_code_t *)(p))

EM_API em_reflist_t em_reflist_code;

#define EM_CODE_INCREF(p) EM_CODE(em_refobj_incref(EM_REFOBJ(p)))
#define EM_CODE_DECREF(p) em_refobj_decref(EM_REFOBJ(p))

/* compiler context */
typedef struct em_code_compiler {
	em_pos_t pos;
	em_code_slice_t *slice;
} em_code_compiler_t;

#define EM_CODE_COMPILER_INIT ((em_code_compiler_t){0})

/* functions */
EM_API em_code_t *em_code_new_node(em_node_t *node, const char *path); /* create code object with node */
EM_API em_code_t *em_code_new_binary(em_code_slice_t binary, const char *path); /* create code object with bytecode slice */
EM_API em_value_t em_code_run(em_code_t *code, struct em_context *context); /* run code */

EM_API void em_code_write_uint8(em_code_slice_t *slice, uint8_t value); /* write uint8 value */
EM_API void em_code_write_uint16(em_code_slice_t *slice, uint16_t value); /* write uint16 value */
EM_API void em_code_write_uint32(em_code_slice_t *slice, uint32_t value); /* write uint32 value */
EM_API void em_code_write_uint64(em_code_slice_t *slice, uint64_t value); /* write uint64 value */
EM_API void em_code_write_int8(em_code_slice_t *slice, int8_t value); /* write int8 value */
EM_API void em_code_write_int16(em_code_slice_t *slice, int16_t value); /* write int16 value */
EM_API void em_code_write_int32(em_code_slice_t *slice, int32_t value); /* write int32 value */
EM_API void em_code_write_int64(em_code_slice_t *slice, int64_t value); /* write int64 value */
EM_API void em_code_write_inttype(em_code_slice_t *slice, em_inttype_t value); /* write emerald int value */
EM_API void em_code_write_float(em_code_slice_t *slice, float value); /* write float value */
EM_API void em_code_write_double(em_code_slice_t *slice, double value); /* write double value */
EM_API void em_code_write_floattype(em_code_slice_t *slice, em_floattype_t value); /* write emerald float value */
EM_API void em_code_write_string(em_code_slice_t *slice, const char *value, size_t len); /* write string */
EM_API void em_code_write_hashed_string(em_code_slice_t *slice, const char *value, size_t len, em_hash_t hash); /* write hashed string */

EM_API uint8_t em_code_read_uint8(em_code_slice_t *slice); /* read uint8 value */
EM_API uint16_t em_code_read_uint16(em_code_slice_t *slice); /* read uint16 value */
EM_API uint32_t em_code_read_uint32(em_code_slice_t *slice); /* read uint32 value */
EM_API uint64_t em_code_read_uint64(em_code_slice_t *slice); /* read uint64 value */
EM_API int8_t em_code_read_int8(em_code_slice_t *slice); /* read int8 value */
EM_API int16_t em_code_read_int16(em_code_slice_t *slice); /* read int16 value */
EM_API int32_t em_code_read_int32(em_code_slice_t *slice); /* read int32 value */
EM_API int64_t em_code_read_int64(em_code_slice_t *slice); /* read int64 value */
EM_API em_inttype_t em_code_read_inttype(em_code_slice_t *slice); /* read emerald int value */
EM_API float em_code_read_float(em_code_slice_t *slice); /* read float value */
EM_API double em_code_read_double(em_code_slice_t *slice); /* read double value */
EM_API em_floattype_t em_code_read_floattype(em_code_slice_t *slice); /* read emerald float value */
EM_API const char *em_code_read_string(em_code_slice_t *slice); /* read string */
EM_API const char *em_code_read_hashed_string(em_code_slice_t *slice, em_hash_t *hash); /* read string with hash */

EM_API size_t em_code_get_size(em_code_compiler_t *compiler, em_node_t *node); /* predict final size of node (useful for branches) */
EM_API void em_code_write(em_code_compiler_t *compiler, em_node_t *node); /* write node */
EM_API void em_code_disassemble(em_code_slice_t *slice, FILE *fp); /* disassemble generated code */

EM_API em_value_t em_code_run_slice(struct em_context *context, em_code_slice_t *slice); /* run code slice */
EM_API void em_code_run_inst(struct em_context *context, em_code_slice_t *slice); /* run single instruction */

#endif /* EMERALD_BYTECODE_H */
