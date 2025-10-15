#!/usr/bin/env emerald
#
# Author: Elliot Kohlmyer
# Date: October 14th, 2025
# Purpose: Test reading and writing files
#
include 'em/os.em'
include 'em/array.em'

let file = os.openFile('obj/test.txt', os.write)
os.writeFile(file, 'Hello, world! Unicode: 水\n')
os.closeFile(file)

let file = os.openFile('obj/test.txt', os.read)
let s = ' ' * 24
os.readFile(file, s)
puts s # Hello, world! Unicode: 水 #
os.closeFile(file)

let a = array.Array(10, array.unsignedChar)
for i = 0 to 10 then
	let a[i] = i + 1
end

let file = os.openFile('obj/test.bin', os.write | os.binary)
os.writeFile(file, a)
os.closeFile(file)

let a = array.Array(10, array.unsignedChar)

let file = os.openFile('obj/test.bin', os.read | os.binary)
os.readFile(file, a)
os.closeFile(file)

# 1 2 3 4 5 6 7 8 9 10 #
for i = 0 to 10 then
	print(a[i])
	print(' ')
end
println('')
