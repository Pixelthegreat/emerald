#!/usr/bin/env emerald
#
# Author: Elliot Kohlmyer
# Date: June 5th, 2026
# Purpose: Test dictionaries
#
include 'em/os.em'
include 'em/dict.em'

puts os.getTrackedMemoryUsage()

let data = dict.Dict({
	'a': 1,
	'b': 2,
	'c': 3
})

let item = none

puts os.getTrackedMemoryUsage() # should be higher than last #

puts data['a'], data['b'], data['c'] # 1 2 3 #

foreach item in dict.iterate(data) then
	puts os.getTrackedMemoryUsage() # should be higher #
	puts item.index, item.key, item.value
end
let item = none

puts os.getTrackedMemoryUsage() # should be lower #
