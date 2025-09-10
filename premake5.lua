workspace 'emerald'
	configurations {'debug', 'release'}

-- global configuration --
em_modules = {
	'site',
}

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

-- core emerald interpreter --
project 'emerald'
	files {'src/emerald/**.c', 'include/emerald/**.h', 'include/emerald.h'}
	includedirs {'obj'}
	links {'m'}
	targetdir 'lib'
	
	defines {'EM_LIB'}

	-- construct module list --
	local em_module_table = 'static em_module_t *modules[] = {\n'
	local em_module_proto = '/* Auto-generated header */\n'

	for i, module in pairs(em_modules) do

		em_module_table = string.format('%s\t&em_module_%s,\n', em_module_table, module)
		em_module_proto = string.format('%sEM_API em_module_t em_module_%s;\n', em_module_proto, module)
	end
	em_module_table = string.format('%s};\n', em_module_table)

	io.writefile('obj/modules.h', string.format('%s\n%s', em_module_proto, em_module_table))

	-- configuration filters --
	filter 'configurations:debug'
		kind 'SharedLib'
	filter 'configurations:release'
		kind 'StaticLib'

-- emerald shell --
project 'shell'
	kind 'ConsoleApp'
	files {'src/shell/**.c', 'include/shell/**.h'}
	links {'emerald', 'm'}
	targetdir 'bin'
