-- Pruebas del motor sin ventana ni GPU: laberintos (ProcGen), vóxeles y SVO (World) y el
-- JobSystem. Ejecutable de consola independiente: compila sólo esos fuentes de Lore (no
-- enlaza GLFW ni la GPU, LORE_HEADLESS), así que corre en cualquier máquina y en CI.
-- Salida 0 = todo pasa. `MazeTests --bench` además mide el SVOBuilder.
project "MazeTests"
	kind "ConsoleApp"
	language "C++"

	targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
	objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

	files {
		"src/**.h",
		"src/**.cpp",
		"../Lore/src/Lore/Log.cpp",
		"../Lore/src/Lore/Core/JobSystem.cpp",
		"../Lore/src/Lore/ProcGen/**.h",
		"../Lore/src/Lore/ProcGen/**.cpp",
		"../Lore/src/Lore/World/SVO.cpp",
		"../Lore/src/Lore/World/VoxelGrid.cpp",
	}

	includedirs {
		"src",
		"../Lore/src",
		"../%{IncludeDirs.spdlog}",
		"../%{IncludeDirs.glm}"
	}

	defines { "LORE_HEADLESS" }

	filter "system:macosx"
		cppdialect "C++latest"
		staticruntime "on"
		architecture 'ARM64'

	filter "system:windows"
		cppdialect "C++latest"
		staticruntime "on"
		architecture 'x64'
		buildoptions { "/utf-8" }

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release or configurations:Dist"
		runtime "Release"
		optimize "on"
