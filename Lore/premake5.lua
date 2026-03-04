project "Lore"
	kind "StaticLib"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "lrpch.h"
	pchsource "src/lrpch.cpp"

	files{
		"src/**.h",
		"src/**.cpp",
		"vendor/glm/glm/**.hpp",
		"vendor/glm/glm/**.inl",
	}

	-- Compile .mm files as Objective-C++ without PCH
	filter "files:**.mm"
		compileas "Objective-C++"
		enablepch "Off"
	filter {}

	includedirs {
		"src",
		"../%{IncludeDirs.spdlog}",
		"../%{IncludeDirs.GLFW}",
		"../%{IncludeDirs.GLAD}",
		"../%{IncludeDirs.IMGUI}",
		"../%{IncludeDirs.glm}"
	}

	filter "system:macosx"
		cppdialect "C++latest"
		staticruntime "on"
		architecture 'ARM64'

		-- Include Objective-C++ files for Metal implementation
		files {
			"src/**.mm"
		}

		defines {
			"LORE_PLATFORM_MAC",
			"_CRT_SECURE_NO_WARNINGS",
			"GLFW_INCLUDE_NONE"
		}

		links {
			"GLFW",
			"GLAD",
			"IMGUI",
			"Cocoa.framework",
			"OpenGL.framework",
			"IOKit.framework",
			"QuartzCore.framework",
			"Metal.framework",
			"MetalKit.framework",
			"Foundation.framework"
		}

		buildoptions { "-fobjc-arc" }

	filter "system:windows"
		cppdialect "C++latest"
		staticruntime "on"
		architecture 'x64'

		links {
			"GLFW",
			"GLAD",
			"IMGUI",
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
