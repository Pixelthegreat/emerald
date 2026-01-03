#!/usr/bin/env emerald
#
# Author: Elliot Kohlmyer
# Date: January 3rd, 2026
# Purpose: Test large lists in Emerald
#
let list = [
	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
]
foreach i in list then end

let list = []
for i = 0 to 100 then
	append(list, i)
end
