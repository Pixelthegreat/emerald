#
# Copyright 2026, Elliot Kohlmyer
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Posix module stub and helpers
#
include 'em/array.em'

let posix = __module_posix

# create map with termios values #
let posix.createTermiosMap = func() then

	return {
		'c_iflag': 0,
		'c_oflag': 0,
		'c_cflag': 0,
		'c_lflag': 0,
		'c_cc': array.Array(posix.NCCS, array.long),
	}
end
