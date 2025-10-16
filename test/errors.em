#!/usr/bin/env emerald
#
# Author: Elliot Kohlmyer
# Date: October 15th, 2025
# Purpose: Test try-catch statement in Emerald
#
class ErrorA of Error then
	func _initialize(this, message) then
		Error._initialize(this, message)
	end
end

class ErrorB of Error then
	func _initialize(this, message) then
		Error._initialize(this, message)
	end
end

try then
	try then
		raise ErrorB('An error')
	catch e = ErrorA then
		puts 'ErrorA'
	end
catch e = ErrorB then
	puts 'ErrorB'
	raise e
end
