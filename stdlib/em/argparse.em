#
# Copyright 2026, Elliot Kohlmyer
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Argument parser module
#
include 'em/string.em'

let argparse = {}

let argparse._printErrorAndExit = func(c, s) then
	puts string.format('{}: {}', c.program, s)
	argparse.printUsage(c)
	exit(1)
end

let argparse._contains = func(l, s) then
	foreach i in l then
		if i == s then
			return true
		end
	end
	return false
end

# Print a usage string #
let argparse.printUsage = func(c) then

	let s = string.format('Usage: {}', c.program)
	if lengthOf(c.arguments) > 0 then

		foreach argument in c.arguments then

			let s = string.format('{} <{}>', s, argument.name)
		end
	end
	if lengthOf(c.options) > 0 or c.includeHelp then

		let s = s + ' [options]'
	end
	puts s
end

# Print full help #
let argparse.printHelp = func(c) then

	argparse.printUsage(c)

	if c.prologue then

		puts '\n' + c.prologue
	end

	# Find length of longest option or argument #
	func getLength(list) then
		let length = 0
		foreach item in list then

			if length then let length = length + 1 end
			let length = length + lengthOf(item)
		end
		return length
	end

	let longest = 0
	foreach option in c.options then

		let length = getLength(option.flags)
		if option.count then
			let length = length + 3 + lengthOf(option.name)
		end

		if length > longest then let longest = length end
	end
	foreach argument in c.arguments then

		let length = lengthOf(argument.name)
		if length > longest then let longest = length end
	end
	if 9 > longest then let longest = 9 end

	# Print options #
	if lengthOf(c.options) > 0 or c.includeHelp then

		puts '\nOptions:'

		if c.includeHelp then

			let s = string.justifyLeft('-h|--help', ' ', longest)
			puts string.format('    {}  Show this help message', s)
		end

		foreach option in c.options then

			let s = ''
			foreach flag in option.flags then

				if s then let s = s + '|' end
				let s = s + flag
			end
			if option.count then
				let s = string.format('{} <{}>', s, option.name)
			end

			let s = string.justifyLeft(s, ' ', longest)
			puts string.format('    {}  {}', s, option.description)
		end
	end

	# Print arguments #
	if lengthOf(c.arguments) > 0 then

		puts '\nArguments:'
		foreach argument in c.arguments then

			let s = string.justifyLeft(argument.name, ' ', longest)
			puts string.format('    {}  {}', s, argument.description)
		end
	end

	if c.epilogue then

		puts '\n' + c.epilogue
	end
end

# Parse arguments #
let argparse.parseArguments = func(argv, c) then

	let table = {}
	let index = -1
	let count = 0

	foreach option in c.options then
		let table[option.name] = none
	end
	foreach argument in c.arguments then
		let table[argument.name] = none
	end

	for i = 1 to lengthOf(argv) then

		let arg = argv[i]

		# Option #
		if lengthOf(arg) and arg[0] == '-' then

			if c.includeHelp and (arg == '-h' or arg == '--help') then

				argparse.printHelp(c)
				exit()

			else then

				let found = none

				foreach option in c.options then

					if argparse._contains(option.flags, arg) then

						let found = option
						break
					end
				end
				if not found then

					argparse._printErrorAndExit(c, string.format('Unrecognized option \'{}\'', arg))

				elif not found.count then

					let table[found.name] = true

				else then

					if not found.append or table[found.name] == none then

						let table[found.name] = []
					end
					for j = 0 to found.count then

						let i = i + 1
						if i >= lengthOf(argv) then

							argparse._printErrorAndExit(c, string.format('Expected {} argument(s) for option \'{}\'', found.count, arg))
						end
						append(table[found.name], argv[i])
					end
				end
			end

		# Argument #
		else then

			if not count then

				let index = index + 1
				if index >= lengthOf(context.arguments) then

					argparse._printErrorAndExit(c, string.format('Unexpected argument \'{}\'', arg))
				end

				let argument = context.arguments[index]

				let count = argument.count
				let table[argument.name] = []
			end

			let argument = context.arguments[index]
			append(table[argument.name], arg)

			if count >= 0 then let count = count - 1 end
		end
	end

	# Insufficient arguments #
	if count >= 0 and (count or index < lengthOf(context.arguments)-1) then

		if index == lengthOf(context.arguments)-2 and context.arguments[index+1].count < 0 then
			return table
		end
		argparse._printErrorAndExit(c, 'Insufficient arguments')
	end
	return table
end
