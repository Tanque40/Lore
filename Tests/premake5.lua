-- Pruebas de los generadores de laberintos. Ejecutable de consola independiente: compila
-- sólo los fuentes de Sandbox/src/Maze (no enlaza Lore, GLFW ni la GPU), así que corre en
-- cualquier máquina y en CI. Salida 0 = todo pasa.
project "MazeTests"
	kind "ConsoleApp"
	language "C++"

	targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
	objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

	files {
		"src/**.cpp",
		"../Sandbox/src/Maze/**.h",
		"../Sandbox/src/Maze/**.cpp",
	}

	includedirs {
		"../Sandbox/src",
		"../%{IncludeDirs.IMGUI}"
	}

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
