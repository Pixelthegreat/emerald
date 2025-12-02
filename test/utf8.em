#!/usr/bin/env emerald
#
# Author: Elliot Kohlmyer
# Date: October 18th, 2025
# Purpose: Test UTF-8 functions
#
include 'em/array.em'
include 'em/utf8.em'

let data = array.Array(4, array.unsignedChar)

puts utf8.encodeInteger(data, 27700) # 3 #
puts data[0], data[1], data[2] # 230 176 180 #

puts utf8.decodeInteger(data) # 27700 #

let data = array.Array(17, array.unsignedChar)

# 72 101 108 108 111 44 32 119 111 114 108 100 33 32 230 176 180 #
let count = utf8.encode(data, 'Hello, world! 水')
for i = 0 to count then

	print(data[i])
	print(' ')
end
println('')

let s = ' ' * 15
puts utf8.decode(s, data) # 17 #
puts s # Hello, world! 水 #

puts utf8.validateBytes(data) # 1 #

let data = array.Array(3, array.unsignedChar)
let data[0] = 255
let data[1] = 128
let data[2] = 14

puts utf8.validateBytes(data) # 0 #
