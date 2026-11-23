#pragma once

#include <Lore.h>

#include "Lore/ProcGen/Maze/Base/Grid.h"
#include "Lore/World/VoxelRenderer.h"

// Lo que sobrevive entre escenas: cada "Generar" crea una MazeScene nueva (y la anterior se
// destruye), pero el renderer (compilar el compute shader cuesta), la cámara y los valores
// de la UI se conservan aquí.
struct SandboxSession {
	Lore::SceneManager* Scenes = nullptr;

	// Se crea en el primer OnAttach (hilo principal), no antes: necesita la API gráfica.
	std::shared_ptr<Lore::VoxelRenderer> Renderer;
	Lore::VoxelRenderSettings RenderSettings;
	Lore::PerspectiveCamera Camera;

	// Última posición del ratón, para que cambiar de escena no haga saltar la cámara.
	float LastMouseX = 0.0f, LastMouseY = 0.0f;
	bool HasLastMouse = false;

	// Valores de la UI
	uint32_t MazeSeed = 1;            // Misma semilla = mismo laberinto (Maze::Random)
	uint32_t GridDimension = 10;      // Laberinto 2D (sólo texto, se genera al instante)
	std::string GridString;
	uint32_t Grid3DDimension = 5;
	uint32_t StalactiteResolution = 256;
	uint32_t StalagmiteResolution = 256;
};
