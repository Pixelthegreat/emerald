#!/usr/bin/env emerald
#
# Author: Elliot Kohlmyer
# Date: April 29th, 2026
# Purpose: Environment for testing memory allocation
#
include 'em/os.em'

func doStuff(value) then
	puts os.getTrackedMemoryUsage()
end

for i = 0 to 10 then
	doStuff(i)
end
