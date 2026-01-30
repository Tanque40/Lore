---@diagnostic disable: undefined-global
workspace "Lore"

configurations { "Debug", "Release", "Dist" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

startproject ("Sandbox")

project "Lore"
	location "Lore"
	kind "SharedLib"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs {
		"%{prj.name}/vendor/spdlog/include",
		"%{prj.name}/src"
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
			"LORE_PLATFORM_WINDOWS",
			"LORE_BUILD_DLL"
		}

		buildoptions { "/utf-8" }

		postbuildcommands {
			("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Sandbox")
		}

	filter "configurations:Debug"
		defines "LR_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "LR_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		defines "LR_DIST"
		optimize "On"

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
		"Lore/vendor/spdlog/include",
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
