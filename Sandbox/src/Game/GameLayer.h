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

	SVO::VoxelGrid m_VoxelGrid{ 64 };
	std::vector<SVO::SVONode> m_SVOData;
	std::shared_ptr<Lore::StorageBuffer> svoBuffer;

	// Compute-First pipeline resources
	std::shared_ptr<Lore::ComputeShader> m_ComputeShader;
	std::shared_ptr<Lore::ComputeTexture> m_ComputeTexture;

	uint32_t m_Width = 800;
	uint32_t m_Height = 600;

	PerspectiveCamera m_Camera;

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

	bool WindowResize(Lore::WindowResizeEvent& e);
};
