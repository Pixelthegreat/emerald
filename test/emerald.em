#!/bin/emerald --
#
# Author: Elliot Kohlmyer
# Date: September 20th, 2025
# Purpose: Test the Emerald programming language
#
func example(a) then
	if a == 'a' then
		return 'b'
	end
	return 'c'
end

puts example('A')
puts example('a')
