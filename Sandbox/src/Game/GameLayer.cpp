#include "sndbxpch.h"

#include "Game/GameLayer.h"

#include <glm/gtc/matrix_transform.hpp>

GameLayer::~GameLayer() {
	m_ComputeShader.reset();
	m_ComputeTexture.reset();
}

void GameLayer::OnAttach() {
	m_Width = Lore::Application::Get().GetWindow().GetWidth();
	m_Height = Lore::Application::Get().GetWindow().GetHeight();
	m_ComputeTexture.reset(Lore::ComputeTexture::Create(m_Width, m_Height));

	glm::vec3 cameraPosition = { 20.0f, 36.0f, 20.0f };
	m_Camera.SetPosition(cameraPosition);
	m_Camera.SetDirection(glm::normalize(glm::vec3(32.0f, 32.0f, 32.0f) - cameraPosition));

	uint32_t squareRatio = 10;
	// Create a visible 8x8x8 cube of voxels centered around (32,32,32)
	for (uint32_t x = 0; x < 36; x++)
		for (uint32_t y = 0; y < 36; y++)
			for (uint32_t z = 0; z < 36; z++) {
				uint32_t squareRoot = static_cast<uint32_t>(std::sqrt((x - 24) * (x - 24) + (y - 24) * (y - 24) + (z - 24) * (z - 24)));
				if (9 < squareRoot && squareRoot <= squareRatio) {
					uint32_t material = rand(); // Un color azul con algo de transparencia
					m_VoxelGrid.SetVoxel(x, y, z, material);
				}
			}

	m_VoxelGrid.SetVoxel(24, 24, 24, 0xFFFFFFFF);

	m_VoxelGrid.SetVoxel(38, 38, 38, 0xFF0000FF); // Red center voxel

	SVO::SVOBuilder builder;
	m_SVOData = builder.Build(m_VoxelGrid);

	uint32_t bufferSize = m_SVOData.size() * sizeof(SVO::SVONode);
	// Lo enlazamos al slot 0 (binding=0)
	svoBuffer.reset(Lore::StorageBuffer::Create(bufferSize, 0));

	// 3. Subes los datos a la GPU
	svoBuffer->SetData(m_SVOData.data(), bufferSize);

	// 4. En el ciclo de renderizado (OnUpdate):

	LR_TRACE("SVO built with {} nodes", m_SVOData.size());
}

void GameLayer::OnUpdate(Lore::TimeStep ts) {
	// Update FPS and frame time
	m_FrameTime = ts.GetSeconds();
	m_FPS = 1.0f / m_FrameTime;

	CameraMovement(ts);
	CameraMouseMovement(ts);

	m_ComputeShader->SetUniform3f("u_CameraPos", m_Camera.GetPosition());
	m_ComputeShader->SetUniform3f("u_CameraDir", m_Camera.GetDirection());
	m_ComputeShader->SetUniform3f("u_CameraUp", m_Camera.GetUp());
	m_ComputeShader->SetUniform3f("u_CameraRight", m_Camera.GetRight());
	m_ComputeShader->SetUniform1f("u_Fov", m_Camera.GetFov());

	svoBuffer->Bind(0);

	// 1. Bind compute resources
	m_ComputeShader->Bind();
	m_ComputeTexture->BindAsImage(0);

	// 2. Dispatch compute shader
	uint32_t groupsX = (m_Width + 15) / 16;
	uint32_t groupsY = (m_Height + 15) / 16;
	Lore::RenderCommand::DispatchCompute(groupsX, groupsY, 1);

	// 3. Memory barrier (ensures compute writes are visible)
	Lore::RenderCommand::ComputeBarrier();

	// 4. Clear and blit the compute texture to screen
	Lore::RenderCommand::SetClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });
	Lore::RenderCommand::Clear();
	Lore::Renderer::BlitToScreen(m_ComputeTexture);
}

void GameLayer::OnEvent(Lore::Event& event) {
	Lore::EventDispatcher dispatcher(event);
	dispatcher.Dispatch<Lore::WindowResizeEvent>(LR_BIND_EVENT_FN(GameLayer::WindowResize));
	dispatcher.Dispatch<Lore::MouseScrolledEvent>(LR_BIND_EVENT_FN(GameLayer::CameraMouseScroll));
}

void GameLayer::CameraMovement(Lore::TimeStep ts) {
	if (Lore::Input::IsKeyPressed(LR_KEY_W))
		m_Camera.ProcessKeyboard(Lore::CameraMovement::FORWARD, ts.GetSeconds());
	if (Lore::Input::IsKeyPressed(LR_KEY_S))
		m_Camera.ProcessKeyboard(Lore::CameraMovement::BACKWARD, ts.GetSeconds());
	if (Lore::Input::IsKeyPressed(LR_KEY_A))
		m_Camera.ProcessKeyboard(Lore::CameraMovement::LEFT, ts.GetSeconds());
	if (Lore::Input::IsKeyPressed(LR_KEY_D))
		m_Camera.ProcessKeyboard(Lore::CameraMovement::RIGHT, ts.GetSeconds());
}

void GameLayer::CameraMouseMovement(Lore::TimeStep ts) {
	auto [mouseX, mouseY] = Lore::Input::GetMousePosition();
	static float lastX = mouseX;
	static float lastY = mouseY;

	float xOffset = mouseX - lastX;
	float yOffset = lastY - mouseY; // Invertimos el eje Y para que subir el ratón mire hacia arriba

	lastX = mouseX;
	lastY = mouseY;

	m_Camera.ProcessMouseMovement(xOffset, yOffset);
}

bool GameLayer::CameraMouseScroll(Lore::MouseScrolledEvent event) {
	float yOffset = event.GetYOffset(); // O la función que tengas para el scroll
	m_Camera.ProcessMouseScroll(yOffset);
	return false;
}

bool GameLayer::WindowResize(Lore::WindowResizeEvent& e) {
	m_Width = e.GetWidth();
	m_Height = e.GetHeight();
	m_ComputeTexture->Resize(m_Width, m_Height);
	return false;
}
