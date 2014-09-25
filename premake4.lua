solution "opengllabs"
    configurations { "Debug", "Release" }
    platforms { "x32", "x64" }
    location "build"
    
    configuration "x32"
        targetdir "bin/x86/"
        debugdir "bin/x86/"
        libdirs "external/lib/x86"
    configuration "x64"
        targetdir "bin/x64/"
        debugdir "bin/x64/"
        libdirs "external/lib/x64"
    configuration "Debug"
        flags { "Symbols" }
    configuration "Release"
        flags { "Optimize" }
    configuration {}
    
    includedirs { "external/include/", "external/include/freetype/", "common/include/" }
    
    
    project "common"
        kind "StaticLib"
        language "C++"
        files { "common/**.h", "common/**.cpp" }
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
        