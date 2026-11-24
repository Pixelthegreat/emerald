#!/usr/bin/env emerald
#
# Author: Elliot Kohlmyer
# Date: November 20th, 2026
# Purpose: Test the experimental bytecode interpreter
#
include 'em/os.em'

let l = [0, 1, 2]
puts let l[0] = 'Hello, world!'

func doStuff() then
	for i = 0 to 1000000 then
		let l[0] = 0
		l[0]
	end
end
puts os.timeOf(doStuff)
