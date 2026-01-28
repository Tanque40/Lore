workspace "Lore"

configurations { "Debug", "Release", "Dist" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

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
		"%{prj.name}/vendor/spdlog/include"
	}

	filter "system:macosx"
		cppdialect "C++latest"
		staticruntime "On"
		architecture 'ARM64'

		defines {
			"LORE_PLATFORM_MAC"
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

	filter "configurations:Debug"
		defines "LR_DEBUG"
		symbols "On"

		filter "configurations:Release"
		defines "LR_RELEASE"
		optimize "On"

		filter "configurations:Dist"
		defines "LR_DIST"
		optimize "On"
