include "dependencies/conandeps.premake5.lua"

workspace "Rasterizer"
    configurations { "Debug", "Release" }
    architecture "x64"

    project "Rasterizer"
        kind "ConsoleApp"
        language "C++"
        cppdialect "C++latest"

        targetdir   "build/%{cfg.buildcfg}/bin"
        objdir      "build/%{cfg.buildcfg}/obj"

        location "./src"
        files { "%{prj.location}/**.h", "%{prj.location}/**.cpp" }
        
        vectorextensions "SSE4.1" 

        -- Debug configuration
        filter "configurations:Debug"
            defines   { "DEBUG" }
            runtime   "Debug"      -- /MDd
            symbols   "On"         -- /Zi + /DEBUG
            optimize  "Off"        -- /Od
        filter {}

        -- Release configuration
        filter "configurations:Release"
            defines   { "NDEBUG" }
            runtime   "Release"    -- /MD
            optimize  "Speed"      -- /O2
            flags     { "LinkTimeOptimization" } -- /GL + /LTCG
        filter {}

        conan_setup()
        linkoptions { "/IGNORE:4099" }
