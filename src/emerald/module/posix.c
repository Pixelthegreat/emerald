/*
 * Copyright 2025, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <emerald/core.h>
#include <emerald/log.h>
#include <emerald/util.h>
#include <emerald/none.h>
#include <emerald/context.h>
#include <emerald/utf8.h>
#include <emerald/string.h>
#include <emerald/module/array.h>
#include <emerald/module/posix.h>

static struct termios original; /* original settings */
static em_bool_t modified_stdin = EM_FALSE; /* tcsetattr modified stdin */

#define DECLARE_FUNCTION(name) em_value_t module_posix_##name(em_context_t *context, em_value_t *args, size_t nargs, em_pos_t *pos)
#define SET_FUNCTION(mod, name) em_util_set_function(mod, #name, module_posix_##name)

#define SET_FLAG(mod, name) em_util_set_value(mod, #name, EM_VALUE_INT(name))

/* posix module */
static em_result_t initialize(em_context_t *context, em_value_t map);
static void destroy(em_context_t *context);

em_module_t em_module_posix = {
	.initialize = initialize,
	.destroy = destroy,
};

/* get error as string */
DECLARE_FUNCTION(strerror) {

	em_inttype_t error = (em_inttype_t)errno;

	if (nargs && em_util_parse_args(pos, args, nargs, "i", &error) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	const char *string = strerror(error);
	if (!string) string = "Unknown error";

	return em_string_new_from_utf8(string, em_utf8_strlen(string));
}

/* read from file */
DECLARE_FUNCTION(read) {

	em_inttype_t fd;
	em_value_t value;
	em_inttype_t count;

	if (em_util_parse_args(pos, args, nargs, "ibi", &fd, &value, &count) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	em_byte_array_t *array = EM_BYTE_ARRAY(EM_OBJECT_FROM_VALUE(value));
	if (array->mode != EM_BYTE_ARRAY_MODE_UNSIGNED_CHAR ||
	    (size_t)count > array->size) {

		em_log_runtime_error(pos, "Invalid arguments");
		return EM_VALUE_FAIL;
	}

	return EM_VALUE_INT((em_inttype_t)read((int)fd, array->data, (size_t)count));
}

/* write to file */
DECLARE_FUNCTION(write) {

	em_inttype_t fd;
	em_value_t value;
	em_inttype_t count;

	if (em_util_parse_args(pos, args, nargs, "ibi", &fd, &value, &count) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	em_byte_array_t *array = EM_BYTE_ARRAY(EM_OBJECT_FROM_VALUE(value));
	if (array->mode != EM_BYTE_ARRAY_MODE_UNSIGNED_CHAR ||
	    (size_t)count >= array->size) {

		em_log_runtime_error(pos, "Invalid arguments");
		return EM_VALUE_FAIL;
	}

	return EM_VALUE_INT((em_inttype_t)write((int)fd, array->data, (size_t)count));
}

/* tcgetattr */
DECLARE_FUNCTION(tcgetattr) {

	em_inttype_t fd;
	em_value_t map;

	if (em_util_parse_args(pos, args, nargs, "im", &fd, &map) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	struct termios attr;
	if (tcgetattr((int)fd, &attr) < 0)
		return EM_VALUE_INT(-1);

	em_util_set_value(map, "c_iflag", EM_VALUE_INT((em_inttype_t)attr.c_iflag));
	em_util_set_value(map, "c_oflag", EM_VALUE_INT((em_inttype_t)attr.c_oflag));
	em_util_set_value(map, "c_cflag", EM_VALUE_INT((em_inttype_t)attr.c_cflag));
	em_util_set_value(map, "c_lflag", EM_VALUE_INT((em_inttype_t)attr.c_lflag));

	/* set control characters */
	em_value_t cc = em_util_get_value(map, "c_cc");
	if (em_is_byte_array(cc)) {

		for (em_ssize_t i = 0; i < NCCS; i++)
			em_byte_array_set(cc, i, (em_inttype_t)attr.c_cc[i]);
	}
	return EM_VALUE_INT(0);
}

/* tcsetattr */
DECLARE_FUNCTION(tcsetattr) {

	em_inttype_t fd;
	em_inttype_t actions;
	em_value_t map;

	if (em_util_parse_args(pos, args, nargs, "iim", &fd, &actions, &map) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	struct termios attr;
	memset(&attr, 0, sizeof(attr));

	em_value_t value = em_util_get_value(map, "c_iflag");
	if (value.type == EM_VALUE_TYPE_INT)
		attr.c_iflag = (tcflag_t)value.value.te_inttype;

	value = em_util_get_value(map, "c_oflag");
	if (value.type == EM_VALUE_TYPE_INT)
		attr.c_oflag = (tcflag_t)value.value.te_inttype;

	value = em_util_get_value(map, "c_cflag");
	if (value.type == EM_VALUE_TYPE_INT)
		attr.c_cflag = (tcflag_t)value.value.te_inttype;

	value = em_util_get_value(map, "c_lflag");
	if (value.type == EM_VALUE_TYPE_INT)
		attr.c_lflag = (tcflag_t)value.value.te_inttype;

	/* get control characters */
	em_value_t cc = em_util_get_value(map, "c_cc");
	if (em_is_byte_array(cc)) {

		for (em_ssize_t i = 0; i < NCCS; i++)
			attr.c_cc[i] = (cc_t)em_byte_array_get(cc, i);
	}

	if (fd == 0) modified_stdin = EM_TRUE;
	return EM_VALUE_INT(tcsetattr((int)fd, (int)actions, &attr));
}

/* initialize module */
static em_result_t initialize(em_context_t *context, em_value_t map) {

	tcgetattr(0, &original);

	em_value_t mod = em_map_new();
	em_util_set_value(map, "__module_posix", mod);

	/* general functions */
	SET_FUNCTION(mod, strerror);

	SET_FUNCTION(mod, read);
	SET_FUNCTION(mod, write);

	/* termios functions */
	SET_FUNCTION(mod, tcgetattr);
	SET_FUNCTION(mod, tcsetattr);

	SET_FLAG(mod, TCSANOW);
	SET_FLAG(mod, TCSADRAIN);
	SET_FLAG(mod, TCSAFLUSH);

	SET_FLAG(mod, IGNBRK);
	SET_FLAG(mod, BRKINT);
	SET_FLAG(mod, IGNPAR);
	SET_FLAG(mod, PARMRK);
	SET_FLAG(mod, INPCK);
	SET_FLAG(mod, ISTRIP);
	SET_FLAG(mod, INLCR);
	SET_FLAG(mod, IGNCR);
	SET_FLAG(mod, ICRNL);
	SET_FLAG(mod, IXON);
	SET_FLAG(mod, IXANY);
	SET_FLAG(mod, IXOFF);

	SET_FLAG(mod, OPOST);
	SET_FLAG(mod, ONLCR);
	SET_FLAG(mod, OCRNL);
	SET_FLAG(mod, ONOCR);
	SET_FLAG(mod, ONLRET);
	SET_FLAG(mod, OFILL);
	SET_FLAG(mod, OFDEL);

	SET_FLAG(mod, CSIZE);
	SET_FLAG(mod, CS5);
	SET_FLAG(mod, CS6);
	SET_FLAG(mod, CS7);
	SET_FLAG(mod, CS8);
	SET_FLAG(mod, CSTOPB);
	SET_FLAG(mod, CREAD);
	SET_FLAG(mod, PARENB);
	SET_FLAG(mod, PARODD);
	SET_FLAG(mod, HUPCL);
	SET_FLAG(mod, CLOCAL);

	SET_FLAG(mod, ISIG);
	SET_FLAG(mod, ICANON);
	SET_FLAG(mod, ECHO);
	SET_FLAG(mod, ECHOE);
	SET_FLAG(mod, ECHOK);
	SET_FLAG(mod, ECHONL);
	SET_FLAG(mod, NOFLSH);
	SET_FLAG(mod, TOSTOP);
	SET_FLAG(mod, IEXTEN);

	SET_FLAG(mod, VEOF);
	SET_FLAG(mod, VEOL);
	SET_FLAG(mod, VERASE);
	SET_FLAG(mod, VINTR);
	SET_FLAG(mod, VKILL);
	SET_FLAG(mod, VMIN);
	SET_FLAG(mod, VQUIT);
	SET_FLAG(mod, VSTART);
	SET_FLAG(mod, VSTOP);
	SET_FLAG(mod, VSUSP);
	SET_FLAG(mod, VTIME);

	SET_FLAG(mod, NCCS);

	return EM_RESULT_SUCCESS;
}

/* destroy module */
static void destroy(em_context_t *context) {

	if (modified_stdin) tcsetattr(0, TCSANOW, &original);
}
