#!/usr/bin/env emerald
#
# Author: Elliot Kohlmyer
# Date: July 10th, 2026
# Purpose: Test program execution time
#
let valueList = [0]

for i = 0 to 1000000 then
	let valueList[0] = i
	valueList[0]
end
