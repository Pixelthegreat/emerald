workspace 'emerald'
	configurations {'debug', 'release'}

-- Options --
newoption {
	trigger = 'enable-asan',
	description = 'Enable address sanitization',
}

-- Global configuration --
em_modules = {
	'array',
	'os',
	'site',
	'string',
}

if _TARGET_OS == 'windows' then
elseif _TARGET_OS == 'eclair' then
else
	table.insert(em_modules, 'posix')
end

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
