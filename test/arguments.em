#!/usr/bin/env emerald
#
# Author: Elliot Kohlmyer
# Date: September 25th, 2025
# Purpose: Test command line arguments
#
include 'em/argparse.em'

let context = {
	'program': 'test/arguments.em',
	'includeHelp': true,

	'prologue': 'This is a test of the argparse module.',
	'epilogue': 'This is a test of the argparse module.',

	'options': [
		{'name': 'warning', 'flags': ['-w', '--warning'], 'count': 0, 'description': 'Show warnings', 'append': false},
		{'name': 'features', 'flags': ['-e', '--enable'], 'count': 1, 'description': 'Enable features', 'append': true},
	],

	'arguments': [
		{'name': 'output', 'count': 1, 'description': 'File output path'},
		{'name': 'inputs', 'count': -1, 'description': 'File input path(s)'},
	],
}
let table = argparse.parseArguments(argv, context)

puts table.warning, table.output[0]

if table.inputs then
	foreach name in table.inputs then
		puts name
	end
end
if table.features then
	foreach name in table.features then
		puts name
	end
end
