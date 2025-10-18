#!/usr/bin/env emerald
#
# Author: Elliot Kohlmyer
# Date: October 16th, 2025
# Purpose: Test (posix-only) terminal control API
#
include 'em/posix.em'

#
# {
#  'c_iflag': 0,
#  'c_oflag': 0,
#  'c_cflag': 0,
#  'c_lflag': 0,
#  'c_cc': array.Array(posix.NCCS, array.long),
# }
#
let map = posix.createTermiosMap()
if posix.tcgetattr(0, map) < 0 then
	raise Error('tcgetattr')
end

let map.c_lflag = map.c_lflag & ~(posix.ECHO)

if posix.tcsetattr(0, posix.TCSANOW, map) < 0 then
	raise Error('tcsetattr')
end

let result = readln('Enter password: ')
puts '\n' + result
