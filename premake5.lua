---@diagnostic disable: undefined-global
workspace "Lore"

configurations { "Debug", "Release", "Dist" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

--- Start project isfor Visual Studio setup, it will open the Sandbox project by default when the solution is loaded
startproject ("Sandbox")

IncludeDirs = {}
IncludeDirs["Lore"] = "Lore/src"
IncludeDirs["spdlog"] = "Lore/vendor/spdlog/include"
IncludeDirs["GLFW"] = "Lore/vendor/GLFW/include"
IncludeDirs["GLAD"] = "Lore/vendor/GLAD/include"
IncludeDirs["IMGUI"] = "Lore/vendor/IMGUI"
IncludeDirs["glm"] = "Lore/vendor/glm"

group "Dependencies"
	include "Lore/vendor/GLFW"
	include "Lore/vendor/GLAD"
	include "Lore/vendor/IMGUI"

group ""
include "Lore"
include "Sandbox"
