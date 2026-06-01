#!/usr/bin/emerald
#
# Author: Elliot Kohlmyer
# Date: June 1st, 2026
# Purpose: Test seeking in files
#
include 'em/io.em'
include 'em/array.em'

let a = array.Array(10, array.unsignedChar)
let a[0] = 10
let a[1] = 20
let a[2] = 30
let a[3] = 40
let a[4] = 50
let a[5] = 60
let a[6] = 70
let a[7] = 80
let a[8] = 90
let a[9] = 100

let b = array.Array(3, array.unsignedChar)
let b[0] = 35
let b[1] = 45
let b[2] = 55

let file = io.open('obj/test.bin', io.write | io.binary)

puts file.write(a) # 10 #
puts file.seek(io.atCursor, -7) # 3 #
puts file.write(b) # 3 #

file.close()
