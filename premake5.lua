workspace 'emerald'
	configurations {'debug', 'release'}

-- global configuration --
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
	kind 'SharedLib'
	files {'src/emerald/**.c', 'include/emerald/**.h', 'include/emerald.h'}
	targetdir 'lib'
	
	defines {'EM_LIB'}

-- emerald shell --
project 'shell'
	kind 'ConsoleApp'
	files {'src/shell/**.c', 'include/shell/**.h'}
	links {'emerald'}
	targetdir 'bin'
