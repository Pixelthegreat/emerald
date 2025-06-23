#!/bin/emerald --
#
# Author: Elliot Kohlmyer
# Date: June 23rd, 2025
# Purpose: Test the Emerald programming language
#
class MyClass then
	func _init(a, b) then
		let this.a = a
		let this.b = b
	end
end

if __name__ == '__main__' then
	puts 'Is main'
elif __name__ == 'module' then
	puts 'Is module'
else then
	puts format('Is {0}', __name__)
end

for i = 0 to 10 then
	puts format('{0}...', i)
end
