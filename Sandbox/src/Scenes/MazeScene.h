#pragma once

#include "Scenes/SandboxSession.h"

#include "Lore/ProcGen/Maze/Base/Grid3D.h"

// Qué mundo construir. Cada botón de la UI arma una receta y pide una MazeScene nueva.
struct WorldRecipe {
	enum class Kind { DemoSphere, Maze3D, Stalactite, Stalagmite };
	enum class Algorithm { BinaryTree, Sidewinder, Wilsons };

	Kind Type = Kind::DemoSphere;
	Algorithm MazeAlgorithm = Algorithm::Wilsons;
	uint32_t MazeSize = 5;     // Celdas por lado (Maze3D)
	uint32_t Seed = 1;         // Maze3D
	uint32_t Resolution = 256; // Stalactite / Stalagmite

	std::string Describe() const;
};

// Escena principal del Sandbox: un mundo de vóxeles (laberinto 3D, espeleotema o la esfera
// de prueba) recorrido con la cámara libre. OnLoad hace todo el trabajo de CPU en segundo
// plano (laberinto -> vóxeles -> SVO) y OnAttach sólo sube el SVO a la GPU.
class MazeScene : public Lore::Scene {
private:
	std::shared_ptr<SandboxSession> m_Session;
	WorldRecipe m_Recipe;

	// Resultado de OnLoad
	std::vector<SVO::SVONode> m_Nodes;
	uint32_t m_WorldSize = 0;
	float m_VoxelScale = 0.25f;
	float m_CameraSpeed = 5.0f;
	glm::vec3 m_StartPosition{ 0.0f };
	glm::vec3 m_StartDirection{ 1.0f, 0.0f, 0.0f };
	std::string m_Grid3DString;
	std::string m_VoxelGridString;
	float m_LoadSeconds = 0.0f;

	// Para el panel de estadísticas
	float m_FPS = 0.0f;
	float m_FrameTime = 0.0f;

	// Con la variable de entorno LORE_LOG_FPS definida, se registra el FPS promedio cada
	// 2 s (para comparar rendimiento entre versiones sin tener que leer el panel).
	bool m_LogFPS = false;
	float m_FPSWindowTime = 0.0f;
	uint32_t m_FPSWindowFrames = 0;
	float m_GpuMsSum = 0.0f;
	uint32_t m_GpuMsSamples = 0;

public:
	MazeScene(std::shared_ptr<SandboxSession> session, WorldRecipe recipe);

	void OnLoad(Lore::LoadContext& context) override;
	void OnAttach() override;
	void OnUpdate(Lore::TimeStep ts) override;
	void OnImGuiRender() override;
	void OnEvent(Lore::Event& event) override;

private:
	void Regenerate(const WorldRecipe& recipe);

	void LoadDemoSphere(Lore::LoadContext& context);
	void LoadMaze3D(Lore::LoadContext& context);
	void LoadSpeleothem(Lore::LoadContext& context, bool stalactite);
	// Construye el SVO de `grid` en la fase [begin, 1] de la barra y libera el grid.
	void BuildSVO(Lore::LoadContext& context, SVO::VoxelGrid& grid, float begin);

	void CameraMovement(Lore::TimeStep ts);
	void CameraMouseMovement();
	bool CameraMouseScroll(Lore::MouseScrolledEvent& event);
	bool WindowResize(Lore::WindowResizeEvent& event);
	bool KeyPressed(Lore::KeyPressedEvent& event);
};
