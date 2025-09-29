#!/usr/bin/env emerald
#
# Author: Elliot Kohlmyer
# Date: September 26th, 2025
# Purpose: Display a progress bar
#
include 'em/os.em'
include 'em/string.em'

let width = 48
let sleepTime = 0.1

for i = 0 to width then

	print(string.format('\r\e[0K[{}{}]', '|' * i, '-' * (width - i)))
	os.sleep(sleepTime)
end
print('\r\e[0K')
