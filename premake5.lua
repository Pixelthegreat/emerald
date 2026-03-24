workspace 'emerald'
	configurations {'debug', 'release'}

-- Split string --
function split(str, delim)
	local result = {}
	local pattern = '([^' .. delim .. ']+)'
	for match in string.gmatch(str, pattern) do
		table.insert(result, match)
	end
	return result
end

-- Index of value in table --
function index_value(table, value)
	for i, v in ipairs(table) do
		if v == value then
			return i
		end
	end
	return -1
end

-- Options --
newoption {
	trigger = 'enable-asan',
	description = 'Enable address sanitization',
}

newoption {
	trigger = 'enable-modules',
	value = 'MODULE1,MODULE2,...',
	description = 'Enable specific modules only (default is all)',
}

newoption {
	trigger = 'disable-modules',
	value = 'MODULE1,MODULE2,...',
	description = 'Disable specific modules',
}

-- Determine module list --
em_modules = {
	'array',
	'os',
	'site',
	'string',
	'utf8',
}

if _TARGET_OS == 'windows' then
elseif _TARGET_OS == 'eclair' then
else
	table.insert(em_modules, 'posix')
end

if _OPTIONS['enable-modules'] then
	em_modules = split(_OPTIONS['enable-modules'], ',')
end

if _OPTIONS['disable-modules'] then
	for _, module in ipairs(split(_OPTIONS['disable-modules'], ',')) do

		index = index_value(em_modules, module)
		if index >= 0 then
			table.remove(em_modules, index)
		end
	end
end

-- Global configuration --
language 'C'
cdialect 'C99'
includedirs {'include'}
objdir 'obj'
libdirs {'lib'}

filter 'configurations:debug'
	symbols 'on'
	defines {'DEBUG'}

filter 'configurations:release'
	optimize 'on'
	defines {'NDEBUG'}

filter 'options:enable-asan'
	sanitize {'Address'}

-- Core emerald interpreter --
project 'emerald'
	files {'src/emerald/*.c', 'include/emerald/*.h', 'include/emerald.h'}
	includedirs {'obj'}
	links {'m'}
	targetdir 'lib'
	
	defines {'EM_LIB'}

	-- Construct module list --
	local em_module_table = 'static em_module_t *modules[] = {\n'
	local em_module_proto = '/* Auto-generated header */\n'

	for i, module in pairs(em_modules) do

		files {string.format('src/emerald/module/%s.c', module)}

		em_module_table = string.format('%s\t&em_module_%s,\n', em_module_table, module)
		em_module_proto = string.format('%sEM_API em_module_t em_module_%s;\n', em_module_proto, module)
	end
	em_module_table = string.format('%s};\n', em_module_table)

	io.writefile('obj/modules.h', string.format('%s\n%s', em_module_proto, em_module_table))

	-- Configuration filters --
	filter 'configurations:debug'
		kind 'SharedLib'
	filter 'configurations:release'
		kind 'StaticLib'

-- Emerald shell --
project 'shell'
	kind 'ConsoleApp'
	files {'src/shell/**.c', 'include/shell/**.h'}
	links {'emerald', 'm'}
	targetdir 'bin'
