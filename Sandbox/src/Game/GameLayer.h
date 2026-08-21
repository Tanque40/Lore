#pragma once

#include <Lore.h>

#include <Lore/Renderer/PerspectiveCamera.h>

#include "Maze/Base/Grid.h"
#include "Maze/Base/Grid3D.h"

#include "SVO/VoxelGrid.h"
#include "SVO/SVO.h"

class GameLayer : public Lore::Layer {
private:
	// For calculating FPS
	float m_FPS = 0.0f;
	float m_FrameTime = 0.0f;

	uint32_t m_GridDimension = 10;
	Maze::Grid m_Grid;
	std::string m_GridString = "";

	uint32_t m_Grid3DDimension = 5;
	Maze::Grid3D m_Grid3D;
	std::string m_Grid3DString = "";
	std::vector<std::vector<std::vector<uint32_t>>> m_Grid3DIntMatrix;
	std::string m_Grid3DIntMatrixString = "";

	SVO::VoxelGrid m_VoxelGrid{ 256 };
	std::string m_VoxelGridString = "";
	SVO::SVOBuilder m_SVOBuilder;

	// Resolución (se redondea a potencia de 2) dedicada por completo a la demo de la
	// estalactita: ver SVO::VoxelGrid::CreateStalactite.
	uint32_t m_StalactiteResolution = 256;

	// Igual que m_StalactiteResolution pero para SVO::VoxelGrid::CreateStalagmite.
	uint32_t m_StalagmiteResolution = 256;
	std::vector<SVO::SVONode> m_SVOData;
	std::shared_ptr<Lore::StorageBuffer> svoBuffer;

	// Tamaño real y profundidad del octree actualmente subido a la GPU (el shader
	// necesita esto para saber cuántos niveles bajar al recorrer el SVO).
	float m_WorldSize = 256.0f;
	float m_MaxLevels = 8.0f;

	// Unidades de mundo por vóxel de índice. Bajarlo hace que los vóxeles se vean
	// más pequeños/finos sin cambiar cuántos hay ni el tamaño del laberinto en índices.
	float m_VoxelScale = 0.25f;

	// LOD por distancia (ver TraceRay en los shaders): a qué fracción tamaño/distancia se
	// deja de bajar por el octree y se renderiza el nodo como un bloque aproximado en vez
	// de resolver el detalle exacto. 0 desactiva el LOD (siempre baja hasta el fondo).
	float m_LodPixelThreshold = 0.02f;
	// Cobertura mínima (0-1) de sólido en el subárbol de un nodo para que, al cortar el
	// descenso por LOD, se dibuje como bloque opaco en vez de tratarse como aire. Bajo a
	// propósito: geometría delgada (cáscaras, paredes finas) tiene poca cobertura en un
	// bloque grande, y un umbral alto (probado con 0.3) la hace desaparecer por completo
	// en vez de verse como un bloque aproximado.
	float m_LodCoverageThreshold = 0.05f;

	// Compute-First pipeline resources
	std::shared_ptr<Lore::ComputeShader> m_ComputeShader;
	std::shared_ptr<Lore::ComputeTexture> m_ComputeTexture;

	uint32_t m_Width = 800;
	uint32_t m_Height = 600;

	float m_MousePositionX, m_MousePositionY;

	Lore::PerspectiveCamera m_Camera;
	float m_CameraSpeed = 10.0f;

public:
	GameLayer() : Layer("Game"), m_Grid(m_GridDimension, m_GridDimension), m_Grid3D(m_Grid3DDimension, m_Grid3DDimension, m_Grid3DDimension) {
		// Initialize compute shader and texture
#ifdef LORE_PLATFORM_MAC
		m_ComputeShader.reset(Lore::ComputeShader::Create("Sandbox/src/Shaders/Metal/Main.compute.metal"));
#elif defined(LORE_PLATFORM_WINDOWS)
		m_ComputeShader.reset(Lore::ComputeShader::Create("src/Shaders/OpenGL/Main.compute.glsl"));
#endif
	}

	~GameLayer();

	void OnAttach() override;
	void OnUpdate(Lore::TimeStep ts) override;
	void OnImGuiRender() override;
	void OnEvent(Lore::Event& event) override;

	void CameraMovement(Lore::TimeStep ts);
	void CameraMouseMovement(Lore::TimeStep ts);

	bool CameraMouseScroll(Lore::MouseScrolledEvent event);
	bool WindowResize(Lore::WindowResizeEvent& event);
	bool KeyPressed(Lore::KeyPressedEvent event);
};
