solution "opengllabs"
    configurations { "Debug", "Release" }
    platforms { "x32", "x64" }
    location "build"
    
	configuration { "x32", "Debug" }
		targetdir "bin/x86/debug/"
        debugdir "bin/x86/debug/"
        libdirs "external/lib/x86/"
		flags { "Symbols" }
	configuration { "x32", "Release" }
		targetdir "bin/x86/release/"
        debugdir "bin/x86/release/"
        libdirs "external/lib/x86/"
		flags { "Optimize" }
	configuration { "x64", "Debug" }
		targetdir "bin/x64/debug/"
        debugdir "bin/x64/debug/"
        libdirs "external/lib/x64/"
		flags { "Symbols" }
	configuration { "x64", "Release" }
		targetdir "bin/x64/release/"
        debugdir "bin/x64/release/"
        libdirs "external/lib/x64/"
		flags { "Optimize" }
	configuration {}
    
    includedirs { "external/include/", "common/include/" }
    
    project "common"
        kind "StaticLib"
        language "C++"
        files { "code/common/**.h", "code/common/**.cpp", "assets/shaders/**.vert", "assets/shaders/**.frag", "assets/shaders/**.geom" }
        objdir "build/common/obj/"
        links { "opengl32", "SDL2", "SDL2main", "glew32" }
        
    project "labs"
        kind "ConsoleApp"
        language "C++"
        files { "code/labs/**.hpp", "code/labs/**.cpp" }
        objdir "build/shadowmapping/obj/"
        links { "opengl32", "SDL2", "SDL2main", "glew32", "common" }
        
    project "project"
        kind "ConsoleApp"
        language "C++"
        files { "code/project/**.hpp", "code/project/**.cpp" }
        objdir "build/project/obj/"
        links { "opengl32", "SDL2", "SDL2main", "glew32", "common" }
        