#!/usr/bin/env emerald
#
# Author: Elliot Kohlmyer
# Date: September 20th, 2025
# Purpose: Test string formatting capabilities
#
include 'em/string.em'

puts string.format('Hello, {0}!', 'world') # -> 'Hello, world!'

puts string.format('\'{}\' {{} {} {1} {} {0}', 'cat', 'dog', 'bird') # -> "'cat' {} dog dog bird cat"

# -> "Error (File '<stdin>', Line 1, Column 1):\n  Fancy error message\n -> line_of_code"
puts string.format('\e[31mError\e[39m (File \'{}\', Line {}, Column {}):\n  Fancy error message\n -> {}', '<stdin>', 1, 1, 'line_of_code')

# End with error #
puts string.format('{} {} {} {}', 'a', 'b', 'c')
