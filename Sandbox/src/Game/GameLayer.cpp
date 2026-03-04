#include "sndbxpch.h"

#include "Game/GameLayer.h"

GameLayer::~GameLayer() {
	m_ComputeShader.reset();
	m_ComputeTexture.reset();
}

void GameLayer::OnAttach() {
	m_Width = Lore::Application::Get().GetWindow().GetWidth();
	m_Height = Lore::Application::Get().GetWindow().GetHeight();
	m_ComputeTexture.reset(Lore::ComputeTexture::Create(m_Width, m_Height));
}

void GameLayer::OnUpdate(Lore::TimeStep ts) {
	// Update FPS and frame time
	m_FrameTime = ts.GetSeconds();
	m_FPS = 1.0f / m_FrameTime;

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
}

bool GameLayer::WindowResize(Lore::WindowResizeEvent& e) {
	m_Width = e.GetWidth();
	m_Height = e.GetHeight();
	m_ComputeTexture->Resize(m_Width, m_Height);
	return false;
}
