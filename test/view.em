#!/usr/bin/env emerald
#
# Author: Elliot Kohlmyer
# Date: June 4th, 2026
# Purpose: Test byte array views
#
include 'em/os.em'
include 'em/array.em'

let data = array.Array(16, array.unsignedChar)
let view = array.View()

for i = 0 to lengthOf(data) then let data[i] = i end

array.setView(view, data, 4, 4)

puts view[0] # 4 #
puts view[2] # 6 #

let view[3] = 19
puts data[7] # 19 #

# These next lines are for keeping memory management in check #
puts os.getTrackedMemoryUsage()

array.setView(view, none, 0, 0)

puts os.getTrackedMemoryUsage() # shouldn't have changed from before #

let view = none

puts os.getTrackedMemoryUsage() # should have changed #
