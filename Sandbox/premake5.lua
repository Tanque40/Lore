project "Sandbox"
	kind "ConsoleApp"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "sndbxpch.h"
	pchsource "src/sndbxpch.cpp"

	files{
		"src/**.h",
		"src/**.cpp",
	}

	includedirs {
		"src",
		"../%{IncludeDirs.spdlog}",
		"../%{IncludeDirs.Lore}",
		"../%{IncludeDirs.glm}",
		"../%{IncludeDirs.IMGUI}"
	}

	links{
		"Lore",
	}

	filter "system:macosx"
		cppdialect "C++latest"
		staticruntime "on"
		architecture 'ARM64'

		defines {
			"LORE_PLATFORM_MAC"
		}

		files {
			"src/**.metal"
		}

		links {
			"IMGUI",
			"GLFW",
			"GLAD",
			"Cocoa.framework",
			"OpenGL.framework",
			"IOKit.framework",
			"QuartzCore.framework",
			"Metal.framework",
			"MetalKit.framework",
			"Foundation.framework"
		}

	filter "system:windows"
		cppdialect "C++latest"
		staticruntime "on"
		architecture 'x64'

		files {
			"src/**.glsl"
		}

		defines {
			"LORE_PLATFORM_WINDOWS",
			"_CRT_SECURE_NO_WARNINGS",
		}

		buildoptions { "/utf-8" }

	filter "configurations:Debug"
		defines{
			"LR_DEBUG",
			"LR_ENABLE_ASSERTS"
		}
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "LR_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "LR_DIST"
		runtime "Release"
		optimize "on"
