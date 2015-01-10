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
    
    includedirs { "external/include/", "external/include/freetype/", "common/include/" }
    
    project "common"
        kind "StaticLib"
        language "C++"
        files { "common/**.h", "common/**.cpp", "assets/shaders/**.vert", "assets/shaders/**.frag" }
        objdir "build/common/obj/"
        links { "opengl32", "SDL2", "SDL2main", "glew32", "freetype" }
    
    project "raytracing"
        kind "ConsoleApp"
        language "C++"
        files { "labs/raytracing/**.hpp", "labs/raytracing/**.cpp" }
        objdir "build/raytracing/obj/"
        links { "opengl32", "SDL2", "SDL2main", "glew32", "freetype", "common" }
        
    project "lighting"
        kind "ConsoleApp"
        language "C++"
        files { "labs/lighting/**.hpp", "labs/lighting/**.cpp" }
        objdir "build/lighting/obj/"
        links { "opengl32", "SDL2", "SDL2main", "glew32", "freetype", "common" }
        