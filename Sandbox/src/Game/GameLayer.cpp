#include "sndbxpch.h"

#include "Game/GameLayer.h"

#include <glm/gtc/matrix_transform.hpp>

GameLayer::~GameLayer() {
	m_ComputeShader.reset();
	m_ComputeTexture.reset();
}

void GameLayer::OnAttach() {
	// El compute shader traza un rayo por píxel REAL del framebuffer (en Retina/HiDPI es
	// mayor que el tamaño de la ventana en puntos), escalado por m_RenderScale.
	m_ComputeTexture.reset(Lore::ComputeTexture::Create(1, 1));
	{
		// Por defecto trazamos tantos píxeles como puntos tiene la ventana (lo mismo que
		// antes de usar el framebuffer real): en Retina eso es 0.5, en una pantalla 1x es
		// 1.0. Subir el slider a 1.0 en Retina da nitidez completa pero 4x más rayos.
		Lore::Window& window = Lore::Application::Get().GetWindow();
		if (window.GetFramebufferWidth() > 0)
			m_RenderScale = std::clamp(static_cast<float>(window.GetWidth()) / static_cast<float>(window.GetFramebufferWidth()), 0.25f, 1.0f);
	}
	ResizeComputeTarget();

	// Posiciones expresadas en índices de vóxel de la escena de muestra, convertidas a
	// unidades de mundo con m_VoxelScale para que la vista quede igual de encuadrada
	// sin importar qué tan finos sean los vóxeles.
	glm::vec3 cameraPosition = glm::vec3(20.0f, 24.0f, 30.0f) * m_VoxelScale;
	m_Camera.SetPosition(cameraPosition);
	m_Camera.SetDirection(glm::normalize(glm::vec3(24.0f, 24.0f, 24.0f) * m_VoxelScale - cameraPosition));

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<unsigned int> distrib(0, 0xFFFFFFFF);

	uint32_t squareRatio = 10;
	// Create a visible 8x8x8 cube of voxels centered around (32,32,32)
	for (uint32_t x = 0; x < 36; x++)
		for (uint32_t y = 0; y < 36; y++)
			for (uint32_t z = 0; z < 36; z++) {
				uint32_t squareRoot = static_cast<uint32_t>(std::sqrt((x - 24) * (x - 24) + (y - 24) * (y - 24) + (z - 24) * (z - 24)));
				if (9 < squareRoot && squareRoot <= squareRatio) {
					uint32_t material = distrib(gen); // Un color aleatorio para cada voxel
					m_VoxelGrid.SetVoxel(x, y, z, material);
				}
			}

	m_VoxelGrid.SetVoxel(24, 24, 24, 0xFFFFFFFF);

	m_VoxelGridString = m_VoxelGrid.ToString();

	m_WorldSize = static_cast<float>(m_VoxelGrid.GetSize());
	m_MaxLevels = std::log2(m_WorldSize);

	m_SVOData = m_SVOBuilder.Build(m_VoxelGrid);
	// El buffer denso de autoría ya no hace falta una vez compactado en el SVO: liberarlo
	// evita mantener el grid completo (potencialmente decenas de MB, casi todo aire) vivo
	// durante el resto de la sesión.
	m_VoxelGrid.Clear();

	uint32_t bufferSize = m_SVOData.size() * sizeof(SVO::SVONode);
	// Lo enlazamos al slot 0 (binding=0)
	svoBuffer.reset(Lore::StorageBuffer::Create(bufferSize, 0));

	// 3. Subes los datos a la GPU
	svoBuffer->SetData(m_SVOData.data(), bufferSize);

	// 4. En el ciclo de renderizado (OnUpdate):

	LR_TRACE("SVO built with {} nodes", m_SVOData.size());

	Lore::Application::Get().GetWindow().HideCursor();
}	

void GameLayer::OnUpdate(Lore::TimeStep ts) {
	// Update FPS and frame time
	m_FrameTime = ts.GetSeconds();
	m_FPS = 1.0f / m_FrameTime;

	CameraMovement(ts);
	CameraMouseMovement(ts);

	svoBuffer->Bind(0);

	// 1. Bind compute resources
	m_ComputeShader->Bind();
	m_ComputeTexture->BindAsImage(0);

	m_ComputeShader->SetUniform3f("u_CameraPos", m_Camera.GetPosition());
	m_ComputeShader->SetUniform3f("u_CameraDir", m_Camera.GetDirection());
	m_ComputeShader->SetUniform3f("u_CameraUp", m_Camera.GetUp());
	m_ComputeShader->SetUniform3f("u_CameraRight", m_Camera.GetRight());
	m_ComputeShader->SetUniform1f("u_Fov", glm::radians(m_Camera.GetFov()));
	m_ComputeShader->SetUniform1f("u_WorldSize", m_WorldSize);
	m_ComputeShader->SetUniform1f("u_MaxLevels", m_MaxLevels);
	m_ComputeShader->SetUniform1f("u_VoxelScale", m_VoxelScale);
	m_ComputeShader->SetUniform1f("u_LodPixelThreshold", m_LodPixelThreshold);
	m_ComputeShader->SetUniform1f("u_LodCoverageThreshold", m_LodCoverageThreshold);

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
	dispatcher.Dispatch<Lore::KeyPressedEvent>(LR_BIND_EVENT_FN(GameLayer::KeyPressed));
}

void GameLayer::CameraMovement(Lore::TimeStep ts) {
	if (Lore::Application::Get().GetWindow().IsCursorHidden()) {
		if (Lore::Input::IsKeyPressed(LR_KEY_W))
			m_Camera.ProcessKeyboard(Lore::CameraMovement::FORWARD, ts.GetSeconds());
		if (Lore::Input::IsKeyPressed(LR_KEY_A))
			m_Camera.ProcessKeyboard(Lore::CameraMovement::LEFT, ts.GetSeconds());
		if (Lore::Input::IsKeyPressed(LR_KEY_S))
			m_Camera.ProcessKeyboard(Lore::CameraMovement::BACKWARD, ts.GetSeconds());
		if (Lore::Input::IsKeyPressed(LR_KEY_D))
			m_Camera.ProcessKeyboard(Lore::CameraMovement::RIGHT, ts.GetSeconds());
	}
}

void GameLayer::CameraMouseMovement(Lore::TimeStep ts) {
	auto [mouseX, mouseY] = Lore::Input::GetMousePosition();
	static float lastX = mouseX;
	static float lastY = mouseY;

	float xOffset = mouseX - lastX;
	float yOffset = lastY - mouseY; // Invertimos el eje Y para que subir el ratón mire hacia arriba

	lastX = mouseX;
	lastY = mouseY;

	if (Lore::Application::Get().GetWindow().IsCursorHidden())
		m_Camera.ProcessMouseMovement(xOffset, yOffset);
}

bool GameLayer::CameraMouseScroll(Lore::MouseScrolledEvent event) {
	float yOffset = event.GetYOffset(); // O la función que tengas para el scroll
	if (Lore::Application::Get().GetWindow().IsCursorHidden())
		m_Camera.ProcessMouseScroll(yOffset);
	return false;
}

bool GameLayer::WindowResize(Lore::WindowResizeEvent& event) {
	// El evento trae el tamaño en puntos; ResizeComputeTarget lee el tamaño real del
	// framebuffer en píxeles (la ventana ya lo actualizó antes de disparar el evento).
	ResizeComputeTarget();
	return false;
}

void GameLayer::ResizeComputeTarget() {
	Lore::Window& window = Lore::Application::Get().GetWindow();
	uint32_t framebufferWidth = window.GetFramebufferWidth();
	uint32_t framebufferHeight = window.GetFramebufferHeight();
	if (framebufferWidth == 0 || framebufferHeight == 0) {
		// Ventana minimizada o tamaño de framebuffer aún no disponible.
		framebufferWidth = window.GetWidth();
		framebufferHeight = window.GetHeight();
	}

	float scale = std::clamp(m_RenderScale, 0.1f, 1.0f);
	m_Width = std::max(1u, static_cast<uint32_t>(framebufferWidth * scale));
	m_Height = std::max(1u, static_cast<uint32_t>(framebufferHeight * scale));
	m_ComputeTexture->Resize(m_Width, m_Height);
}

void GameLayer::GenerateMaze3D(void (*algorithm)(Maze::Grid3D*, Maze::Random&)) {
	m_Grid3D = Maze::Grid3D(m_Grid3DDimension, m_Grid3DDimension, m_Grid3DDimension);
	Maze::Random rng(m_MazeSeed);
	algorithm(&m_Grid3D, rng);
	m_Grid3DString = m_Grid3D.ToString();

	m_VoxelGrid = m_VoxelGrid.CastToVoxels(&m_Grid3D, 256);

	m_WorldSize = static_cast<float>(m_VoxelGrid.GetSize());
	m_MaxLevels = std::log2(m_WorldSize);

	m_SVOData = m_SVOBuilder.Build(m_VoxelGrid);
	m_VoxelGrid.Clear();

	uint32_t bufferSize = static_cast<uint32_t>(m_SVOData.size() * sizeof(SVO::SVONode));
	// Lo enlazamos al slot 0 (binding=0)
	svoBuffer.reset(Lore::StorageBuffer::Create(bufferSize, 0));
	svoBuffer->SetData(m_SVOData.data(), bufferSize);

	// Fuera del laberinto todo es piedra sólida: la cámara tiene que arrancar DENTRO del
	// primer pasillo tallado (la celda de entrada), no en su posición libre anterior, o
	// quedaría embebida en roca.
	Maze::Cell3D* entryCell = m_Grid3D(0, 0, 0);
	float entryIndexCenter = static_cast<float>(SVO::VoxelGrid::kCellsPerAxis) * 0.5f;
	glm::vec3 entryVoxelCenter = glm::vec3(entryIndexCenter) * m_VoxelScale;
	glm::vec3 lookDir = { 1.0f, 0.0f, 0.0f };
	if (entryCell->IsLinked(entryCell->GetEast())) {
		lookDir = { 1.0f, 0.0f, 0.0f };
	} else if (entryCell->IsLinked(entryCell->GetUp())) {
		lookDir = { 0.0f, 1.0f, 0.0f };
	} else if (entryCell->IsLinked(entryCell->GetSouth())) {
		lookDir = { 0.0f, 0.0f, 1.0f };
	}
	m_Camera.SetPosition(entryVoxelCenter);
	m_Camera.SetDirection(lookDir);
}

bool GameLayer::KeyPressed(Lore::KeyPressedEvent event) {
	if (event.GetKeyCode() == LR_KEY_ESCAPE) {
		ImGui::SetWindowFocus(NULL);
		Lore::Application::Get().GetWindow().ShowCursor();
	}
	return false;
}
