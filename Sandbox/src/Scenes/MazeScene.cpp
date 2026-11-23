#include "sndbxpch.h"

#include "Scenes/MazeScene.h"

#include "Lore/ProcGen/Maze/Algorithms/BinaryTree.h"
#include "Lore/ProcGen/Maze/Algorithms/Sidewinder.h"
#include "Lore/ProcGen/Maze/Algorithms/Wilsons.h"

namespace {

	const char* AlgorithmName(WorldRecipe::Algorithm algorithm) {
		switch (algorithm) {
		case WorldRecipe::Algorithm::BinaryTree: return "Binary Tree";
		case WorldRecipe::Algorithm::Sidewinder: return "Sidewinder";
		case WorldRecipe::Algorithm::Wilsons:    return "Wilson's";
		}
		return "?";
	}

	// Ruta del compute shader relativa a la raíz del repo (desde donde se corre el
	// ejecutable en Mac) o a Sandbox/ (directorio de trabajo de Visual Studio).
#ifdef LORE_PLATFORM_MAC
	constexpr const char* kComputeShaderPath = "Sandbox/src/Shaders/Metal/Main.compute.metal";
#else
	constexpr const char* kComputeShaderPath = "src/Shaders/OpenGL/Main.compute.glsl";
#endif

}

std::string WorldRecipe::Describe() const {
	switch (Type) {
	case Kind::DemoSphere: return "esfera de prueba";
	case Kind::Maze3D:     return std::string("laberinto ") + AlgorithmName(MazeAlgorithm) + " " + std::to_string(MazeSize) + "^3 (semilla " + std::to_string(Seed) + ")";
	case Kind::Stalactite: return "estalactita " + std::to_string(Resolution) + "^3";
	case Kind::Stalagmite: return "estalagmita " + std::to_string(Resolution) + "^3";
	}
	return "mundo";
}

MazeScene::MazeScene(std::shared_ptr<SandboxSession> session, WorldRecipe recipe)
	: Scene(recipe.Describe()), m_Session(std::move(session)), m_Recipe(recipe) {
}

// =========================================================================================
// Carga (hilo del JobSystem): sólo CPU
// =========================================================================================

void MazeScene::OnLoad(Lore::LoadContext& context) {
	switch (m_Recipe.Type) {
	case WorldRecipe::Kind::DemoSphere: LoadDemoSphere(context); break;
	case WorldRecipe::Kind::Maze3D:     LoadMaze3D(context); break;
	case WorldRecipe::Kind::Stalactite: LoadSpeleothem(context, true); break;
	case WorldRecipe::Kind::Stalagmite: LoadSpeleothem(context, false); break;
	}
}

void MazeScene::BuildSVO(Lore::LoadContext& context, SVO::VoxelGrid& grid, float begin) {
	if (context.IsCancelled() || grid.GetSize() == 0)
		return;

	m_WorldSize = grid.GetSize();
	context.SetStage("Construyendo SVO");
	SVO::SVOBuilder builder;
	m_Nodes = builder.Build(grid, context.Phase(begin, 1.0f));
	// El grid denso ya no hace falta una vez compactado en el SVO: liberarlo evita mantener
	// decenas de MB (casi todo aire) vivos durante el resto de la sesión.
	grid.Clear();
	LR_TRACE("SVO construido con {} nodos", m_Nodes.size());
}

void MazeScene::LoadDemoSphere(Lore::LoadContext& context) {
	context.SetStage("Generando esfera de prueba");
	SVO::VoxelGrid grid(256);

	std::mt19937 gen(std::random_device{}());
	std::uniform_int_distribution<unsigned int> distrib(0, 0xFFFFFFFF);

	// Cáscara esférica de radio ~10 centrada en (24,24,24), un color aleatorio por vóxel.
	uint32_t squareRatio = 10;
	for (uint32_t x = 0; x < 36; x++)
		for (uint32_t y = 0; y < 36; y++)
			for (uint32_t z = 0; z < 36; z++) {
				uint32_t squareRoot = static_cast<uint32_t>(std::sqrt((x - 24) * (x - 24) + (y - 24) * (y - 24) + (z - 24) * (z - 24)));
				if (9 < squareRoot && squareRoot <= squareRatio)
					grid.SetVoxel(x, y, z, distrib(gen));
			}
	grid.SetVoxel(24, 24, 24, 0xFFFFFFFF);
	m_VoxelGridString = grid.ToString();

	m_VoxelScale = 0.25f;
	m_CameraSpeed = 5.0f;
	// Posiciones expresadas en índices de vóxel, convertidas a unidades de mundo con
	// m_VoxelScale para que la vista quede igual de encuadrada sin importar el tamaño.
	m_StartPosition = glm::vec3(20.0f, 24.0f, 30.0f) * m_VoxelScale;
	m_StartDirection = glm::normalize(glm::vec3(24.0f, 24.0f, 24.0f) * m_VoxelScale - m_StartPosition);

	BuildSVO(context, grid, 0.2f);
}

void MazeScene::LoadMaze3D(Lore::LoadContext& context) {
	context.SetStage("Generando laberinto");
	uint32_t n = std::max(1u, m_Recipe.MazeSize);
	Maze::Grid3D maze(n, n, n);
	Maze::Random rng(m_Recipe.Seed);
	switch (m_Recipe.MazeAlgorithm) {
	case WorldRecipe::Algorithm::BinaryTree: Maze::BinaryTree::On(&maze, rng); break;
	case WorldRecipe::Algorithm::Sidewinder: Maze::Sidewinder::On(&maze, rng); break;
	case WorldRecipe::Algorithm::Wilsons:    Maze::Wilsons::On(&maze, rng); break;
	}
	m_Grid3DString = maze.ToString();
	context.SetProgress(0.05f);
	if (context.IsCancelled()) return;

	context.SetStage("Tallando la cueva");
	SVO::VoxelGrid grid = SVO::VoxelGrid::CastToVoxels(&maze, 256, context.Phase(0.05f, 0.7f));

	// Fuera del laberinto todo es piedra sólida: la cámara tiene que arrancar DENTRO del
	// primer pasillo tallado (la celda de entrada), o quedaría embebida en roca.
	m_VoxelScale = 0.25f;
	m_CameraSpeed = 5.0f;
	Maze::Cell3D* entryCell = maze(0, 0, 0);
	float entryIndexCenter = static_cast<float>(SVO::VoxelGrid::kCellsPerAxis) * 0.5f;
	m_StartPosition = glm::vec3(entryIndexCenter) * m_VoxelScale;
	m_StartDirection = { 1.0f, 0.0f, 0.0f };
	if (entryCell->IsLinked(entryCell->GetEast()))
		m_StartDirection = { 1.0f, 0.0f, 0.0f };
	else if (entryCell->IsLinked(entryCell->GetUp()))
		m_StartDirection = { 0.0f, 1.0f, 0.0f };
	else if (entryCell->IsLinked(entryCell->GetSouth()))
		m_StartDirection = { 0.0f, 0.0f, 1.0f };

	BuildSVO(context, grid, 0.7f);
}

void MazeScene::LoadSpeleothem(Lore::LoadContext& context, bool stalactite) {
	context.SetStage(stalactite ? "Generando estalactita" : "Generando estalagmita");
	SVO::VoxelGrid grid = stalactite
		? SVO::VoxelGrid::CreateStalactite(m_Recipe.Resolution)
		: SVO::VoxelGrid::CreateStalagmite(m_Recipe.Resolution);
	m_VoxelGridString = grid.ToString();
	context.SetProgress(0.5f);

	// El espeleotema es un objeto pequeño: encogemos voxelScale para que ocupe siempre ~6
	// unidades de mundo sin importar la resolución, así subirla se traduce en vóxeles más
	// chicos sobre el mismo objeto (no en un mundo más grande).
	float worldSize = static_cast<float>(grid.GetSize());
	m_VoxelScale = 6.0f / worldSize;
	m_CameraSpeed = 1.0f; // El objeto es chico: la velocidad por defecto lo sobrepasaría

	// El ray marcher no recorta el rayo contra el AABB del mundo: si la cámara arranca
	// afuera de [0, worldSize] no se ve nada. Por eso queda DENTRO de esos límites.
	float extent = worldSize * m_VoxelScale;
	glm::vec3 lookTarget(extent * 0.5f, extent * 0.6f, extent * 0.5f);
	m_StartPosition = glm::vec3(extent * 0.15f, extent * 0.2f, extent * 0.15f);
	m_StartDirection = glm::normalize(lookTarget - m_StartPosition);

	BuildSVO(context, grid, 0.5f);
}

// =========================================================================================
// Hilo principal
// =========================================================================================

void MazeScene::OnAttach() {
	// La primera escena crea el renderer y captura el ratón para la cámara (como hacía
	// GameLayer al arrancar); las regeneraciones posteriores dejan el cursor como esté.
	bool firstScene = !m_Session->Renderer;
	if (firstScene)
		m_Session->Renderer = std::make_shared<Lore::VoxelRenderer>(kComputeShaderPath);

	// Si la ventana cambió de tamaño durante la carga, el evento lo recibió la pantalla de
	// carga y no nosotros.
	m_Session->Renderer->Resize();
	m_Session->Renderer->Upload(m_Nodes, m_WorldSize);
	m_Nodes.clear();
	m_Nodes.shrink_to_fit();
	m_Session->RenderSettings.VoxelScale = m_VoxelScale;

	Lore::PerspectiveCamera& camera = m_Session->Camera;
	camera.SetPosition(m_StartPosition);
	camera.SetDirection(m_StartDirection);
	*camera.GetMovemetSpeedPtr() = m_CameraSpeed;

	m_LoadSeconds = m_Session->Scenes ? m_Session->Scenes->GetLastLoadSeconds() : 0.0f;

	if (firstScene)
		Lore::Application::Get().GetWindow().HideCursor();
}

void MazeScene::Regenerate(const WorldRecipe& recipe) {
	m_Session->Scenes->LoadAsync(std::make_unique<MazeScene>(m_Session, recipe));
}

void MazeScene::OnUpdate(Lore::TimeStep ts) {
	m_FrameTime = ts.GetSeconds();
	m_FPS = m_FrameTime > 0.0f ? 1.0f / m_FrameTime : 0.0f;

	CameraMovement(ts);
	CameraMouseMovement();

	m_Session->Renderer->Render(m_Session->Camera, m_Session->RenderSettings);
}

void MazeScene::OnEvent(Lore::Event& event) {
	Lore::EventDispatcher dispatcher(event);
	dispatcher.Dispatch<Lore::WindowResizeEvent>(LR_BIND_EVENT_FN(MazeScene::WindowResize));
	dispatcher.Dispatch<Lore::MouseScrolledEvent>(LR_BIND_EVENT_FN(MazeScene::CameraMouseScroll));
	dispatcher.Dispatch<Lore::KeyPressedEvent>(LR_BIND_EVENT_FN(MazeScene::KeyPressed));
}

void MazeScene::CameraMovement(Lore::TimeStep ts) {
	if (!Lore::Application::Get().GetWindow().IsCursorHidden())
		return;

	Lore::PerspectiveCamera& camera = m_Session->Camera;
	if (Lore::Input::IsKeyPressed(LR_KEY_W))
		camera.ProcessKeyboard(Lore::CameraMovement::FORWARD, ts.GetSeconds());
	if (Lore::Input::IsKeyPressed(LR_KEY_A))
		camera.ProcessKeyboard(Lore::CameraMovement::LEFT, ts.GetSeconds());
	if (Lore::Input::IsKeyPressed(LR_KEY_S))
		camera.ProcessKeyboard(Lore::CameraMovement::BACKWARD, ts.GetSeconds());
	if (Lore::Input::IsKeyPressed(LR_KEY_D))
		camera.ProcessKeyboard(Lore::CameraMovement::RIGHT, ts.GetSeconds());
}

void MazeScene::CameraMouseMovement() {
	auto [mouseX, mouseY] = Lore::Input::GetMousePosition();
	SandboxSession& session = *m_Session;
	if (!session.HasLastMouse) {
		session.LastMouseX = mouseX;
		session.LastMouseY = mouseY;
		session.HasLastMouse = true;
	}

	float xOffset = mouseX - session.LastMouseX;
	float yOffset = session.LastMouseY - mouseY; // Invertimos Y para que subir el ratón mire hacia arriba
	session.LastMouseX = mouseX;
	session.LastMouseY = mouseY;

	if (Lore::Application::Get().GetWindow().IsCursorHidden())
		session.Camera.ProcessMouseMovement(xOffset, yOffset);
}

bool MazeScene::CameraMouseScroll(Lore::MouseScrolledEvent& event) {
	if (Lore::Application::Get().GetWindow().IsCursorHidden())
		m_Session->Camera.ProcessMouseScroll(event.GetYOffset());
	return false;
}

bool MazeScene::WindowResize(Lore::WindowResizeEvent& event) {
	// El evento trae el tamaño en puntos; Resize lee el framebuffer real en píxeles (la
	// ventana ya lo actualizó antes de disparar el evento).
	m_Session->Renderer->Resize();
	return false;
}

bool MazeScene::KeyPressed(Lore::KeyPressedEvent& event) {
	if (event.GetKeyCode() == LR_KEY_ESCAPE) {
		ImGui::SetWindowFocus(NULL);
		Lore::Application::Get().GetWindow().ShowCursor();
	}
	return false;
}
