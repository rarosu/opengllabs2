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
    
    includedirs { "external/include/", "code/common/include/" }
    
    project "common"
        kind "StaticLib"
        language "C++"
        files { "code/common/**.h", "code/common/**.cpp", "assets/shaders/**.vert", "assets/shaders/**.frag", "assets/shaders/**.geom" }
        objdir "build/common/obj/"
        links { "opengl32", "SDL2", "SDL2main", "gl3w" }
    
    project "raytracing"
        kind "ConsoleApp"
        language "C++"
        files { "code/raytracing/**.hpp", "code/raytracing/**.cpp", "code/raytracing/shaders/**.vert", "code/raytracing/shaders/**.frag" }
        objdir "build/raytracing/obj/"
        links { "opengl32", "SDL2", "SDL2main", "gl3w", "common" }
        
    project "lighting"
        kind "ConsoleApp"
        language "C++"
        files { "code/lighting/**.hpp", "code/lighting/**.cpp", "code/lighting/shaders/**.vert", "code/lighting/shaders/**.frag" }
        objdir "build/lighting/obj/"
        links { "opengl32", "SDL2", "SDL2main", "gl3w", "common" }
        
    project "project"
        kind "ConsoleApp"
        language "C++"
        files { "code/project/**.hpp", "code/project/**.cpp", "code/project/shaders/**.vert", "code/project/shaders/**.frag", "code/project/shaders/**.geom" }
        objdir "build/project/obj/"
        links { "opengl32", "SDL2", "SDL2main", "gl3w", "common" }
        
    project "objviewer"
        kind "ConsoleApp"
        language "C++"
        files { "code/objviewer/**.hpp", "code/objviewer/**.cpp", "code/objviewer/shaders/**.vert", "code/objviewer/shaders/**.frag" }
        objdir "build/objviewer/obj/"
        links { "opengl32", "SDL2", "SDL2main", "gl3w", "common" }
        
    project "shadowmapping"
        kind "ConsoleApp"
        language "C++"
        files { "code/shadowmapping/**.hpp", "code/shadowmapping/**.cpp", "code/shadowmapping/shaders/**.vert", "code/shadowmapping/shaders/**.frag" }
        objdir "build/shadowmapping/obj/"
        links { "opengl32", "SDL2", "SDL2main", "gl3w", "common" }
        