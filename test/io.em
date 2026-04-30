#!/usr/bin/emerald
#
# Author: Elliot Kohlmyer
# Date: April 3rd, 2026
# Purpose: Test IO module
#
include 'em/io.em'
include 'em/array.em'

let file = io.open('obj/test.txt', io.write)
file.write('Hello, world! Unicode: 水\n')
file.close()

let file = io.open('obj/test.txt', io.read)
puts file.readString(24)
file.close()

let a = array.Array(10, array.unsignedChar)
for i = 0 to 10 then
	let a[i] = i + 1
end

let file = io.open('obj/test.bin', io.write | io.binary)
file.write(a)
file.close()

let a = array.Array(10, array.unsignedChar)

let file = io.open('obj/test.bin', io.read | io.binary)
file.read(a)
file.close()

for i = 0 to 10 then
	print(a[i])
	print(' ')
end
println('')
