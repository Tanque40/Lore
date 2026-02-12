---@diagnostic disable: undefined-global
workspace "Lore"

configurations { "Debug", "Release", "Dist" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

startproject ("Sandbox")

IncludeDirs = {}
IncludeDirs["spdlog"] = "Lore/vendor/spdlog/include"
IncludeDirs["GLFW"] = "Lore/vendor/GLFW/include"

include "Lore/vendor/GLFW"
--- Start project isfor Visual Studio setup, it will open the Sandbox project by default when the solution is loaded

project "Lore"
	location "Lore"
	kind "SharedLib"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "lrpch.h"
	pchsource "Lore/src/lrpch.cpp"

	files{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs {
		"%{prj.name}/src",
		"%{IncludeDirs.spdlog}",
		"%{IncludeDirs.GLFW}"
	}

	filter "system:macosx"
		cppdialect "C++latest"
		staticruntime "On"
		architecture 'ARM64'

		defines {
			"LORE_PLATFORM_MAC",
			"_CRT_SECURE_NO_WARNINGS",
			"GLFW_INCLUDE_NONE"
		}

		links {
			"GLFW",
			"Cocoa.framework",
			"OpenGL.framework",
			"IOKit.framework",
			"QuartzCore.framework"
		}

	filter "system:windows"
		cppdialect "C++latest"
		staticruntime "On"
		architecture 'x64'

		links {
			"GLFW",
			"opengl32.lib",
		}

		defines {
			"LORE_PLATFORM_WINDOWS",
			"LORE_BUILD_DLL",
			"_CRT_SECURE_NO_WARNINGS",
			"_GLFW_WIN32",
			"GLFW_INCLUDE_NONE",
		}

		buildoptions { "/utf-8" }

		postbuildcommands {
			("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Sandbox")
		}

	filter "configurations:Debug"
		defines{
			"LR_DEBUG",
			"LR_ENABLE_ASSERTS"
		}
		symbols "On"

	filter "configurations:Release"
		defines "LR_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		defines "LR_DIST"
		optimize "On"

	-- Filtros ESPECÍFICOS para Windows para agregar las banderas /MD
	filter { "system:windows", "configurations:Debug" }
		buildoptions "/MDd"

	filter { "system:windows", "configurations:Release" }
		buildoptions "/MD"

	filter { "system:windows", "configurations:Dist" }
		buildoptions "/MD"

project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs {
		"%{IncludeDirs.spdlog}",
		"Lore/src"
	}

	links{
		"Lore"
	}

	filter "system:macosx"
		cppdialect "C++latest"
		staticruntime "On"
		architecture 'ARM64'

		defines {
			"LORE_PLATFORM_MAC"
		}

	filter "system:windows"
		cppdialect "C++latest"
		staticruntime "On"
		architecture 'x64'

		defines {
			"LORE_PLATFORM_WINDOWS"
		}

		buildoptions { "/utf-8" }

	filter "configurations:Debug"
		defines "LR_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "LR_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		defines "LR_DIST"
		optimize "On"
