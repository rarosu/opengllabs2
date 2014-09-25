solution "opengllabs"
    configurations { "Debug", "Release" }
    platforms { "x32", "x64" }
    location "build"
    
    configuration "x32"
        targetdir "bin/x86/"
        libdirs "external/lib/x86"
    configuration "x64"
        targetdir "bin/x64/"
        libdirs "external/lib/x64"
    configuration {}
    
    includedirs "external/include/"
    links { "SDL2", "SDL2main", "glew32" }
    
    project "raytracing"
        kind "ConsoleApp"
        language "C++"
        files { "labs/raytracing/**.hpp", "labs/raytracing/**.cpp" }
        objdir "build/raytracing/obj/"