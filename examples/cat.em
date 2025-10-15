#!/usr/bin/env emerald
#
# Author: Elliot Kohlmyer
# Date: October 14th, 2025
# Purpose: Example similar to the 'cat' utility
#
include 'em/os.em'
include 'em/argparse.em'

let context = {
	'program': 'examples/cat.em',
	'includeHelp': true,

	'prologue': none,
	'epilogue': none,

	'options': [],
	'arguments': [
		{'name': 'input', 'count': 1, 'description': 'File input path'},
	],
}
let table = argparse.parseArguments(argv, context)

let file = os.openFile(table.input[0], os.read)

let buffer = ' ' * 256
let nread = lengthOf(buffer)

while nread == lengthOf(buffer) then

	let nread = os.readFile(file, buffer)
	for i = 0 to nread then
		print(buffer[i])
	end
end
os.closeFile(file)
