/*
 * Copyright 2025, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef EMERALD_OBJECT_H
#define EMERALD_OBJECT_H

#include <emerald/core.h>
#include <emerald/value.h>
#include <emerald/refobj.h>

struct em_object;

/* object type */
typedef struct em_object_type {
	em_value_t (*is_true)(em_value_t, em_pos_t *);
	em_value_t (*add)(em_value_t, em_value_t, em_pos_t *);
	em_value_t (*subtract)(em_value_t, em_value_t, em_pos_t *);
	em_value_t (*multiply)(em_value_t, em_value_t, em_pos_t *);
	em_value_t (*divide)(em_value_t, em_value_t, em_pos_t *);
	em_value_t (*or)(em_value_t, em_value_t, em_pos_t *);
	em_value_t (*and)(em_value_t, em_value_t, em_pos_t *);
	em_value_t (*shift_left)(em_value_t, em_value_t, em_pos_t *);
	em_value_t (*shift_right)(em_value_t, em_value_t, em_pos_t *);
	em_value_t (*compare_equal)(em_value_t, em_value_t, em_pos_t *);
	em_value_t (*compare_less_than)(em_value_t, em_value_t, em_pos_t *);
	em_value_t (*compare_greater_than)(em_value_t, em_value_t, em_pos_t *);
	em_value_t (*to_string)(em_value_t, em_pos_t *);
} em_object_type_t;

/* object */
typedef struct em_object {
	em_refobj_t base;
	em_object_type_t *type; /* object type */
} em_object_t;

#define EM_OBJECT(p) ((em_object_t *)(p))

EM_API em_reflist_t em_reflist_object;

#define EM_OBJECT_INCREF(p) EM_OBJECT(em_refobj_incref(EM_REFOBJ(p)))
#define EM_OBJECT_DECREF(p) em_refobj_decref(EM_REFOBJ(p))

#define EM_OBJECT_AS_VALUE(p) ((em_value_t){.type = EM_VALUE_TYPE_OBJECT, .value.t_voidp = (void *)(p)})
#define EM_OBJECT_FROM_VALUE(v) EM_OBJECT((v).value.t_voidp)

/* functions */
EM_API em_value_t em_object_new(em_object_type_t *type, size_t size); /* create object */
EM_API em_value_t em_object_is_true(em_value_t v, em_pos_t *pos); /* get truthiness of object */
EM_API em_value_t em_object_add(em_value_t a, em_value_t b, em_pos_t *pos); /* add objects */
EM_API em_value_t em_object_subtract(em_value_t a, em_value_t b, em_pos_t *pos); /* subtract objects */
EM_API em_value_t em_object_multiply(em_value_t a, em_value_t b, em_pos_t *pos); /* multiply objects */
EM_API em_value_t em_object_divide(em_value_t a, em_value_t b, em_pos_t *pos); /* divide objects */
EM_API em_value_t em_object_or(em_value_t a, em_value_t b, em_pos_t *pos); /* or objects */
EM_API em_value_t em_object_and(em_value_t a, em_value_t b, em_pos_t *pos); /* and objects */
EM_API em_value_t em_object_shift_left(em_value_t a, em_value_t b, em_pos_t *pos); /* shift left */
EM_API em_value_t em_object_shift_right(em_value_t a, em_value_t b, em_pos_t *pos); /* shift right */
EM_API em_value_t em_object_compare_equal(em_value_t a, em_value_t b, em_pos_t *pos); /* compare if objects are equal */
EM_API em_value_t em_object_compare_less_than(em_value_t a, em_value_t b, em_pos_t *pos); /* compare if object is less than other */
EM_API em_value_t em_object_compare_greater_than(em_value_t a, em_value_t b, em_pos_t *pos); /* compare if object is greater than other */
EM_API em_value_t em_object_to_string(em_value_t v, em_pos_t *pos); /* get string representation of object */

#endif /* EMERALD_OBJECT_H */
