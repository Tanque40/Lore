#include "sndbxpch.h"

#include "Scenes/MazeScene.h"

#include "Lore/ProcGen/Maze/Algorithms/BinaryTree.h"
#include "Lore/ProcGen/Maze/Algorithms/Sidewinder.h"
#include "Lore/ProcGen/Maze/Algorithms/Wilsons.h"

namespace {

	void ScrollingText(const char* id, const std::string& text, int maxHeightInLines) {
		ImGui::SetNextWindowSizeConstraints(ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * 1), ImVec2(FLT_MAX, ImGui::GetTextLineHeightWithSpacing() * maxHeightInLines));
		ImGui::BeginChild(id, ImVec2(-FLT_MIN, 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY, ImGuiWindowFlags_HorizontalScrollbar);
		ImGui::Text("%s", text.c_str());
		ImGui::EndChild();
	}

}

void MazeScene::OnImGuiRender() {
	SandboxSession& session = *m_Session;
	Lore::VoxelRenderer& renderer = *session.Renderer;
	Lore::PerspectiveCamera& camera = session.Camera;

	Lore::Application& app = Lore::Application::Get();
	Lore::FramebufferSpecification spec = app.GetFramebuffer().GetSpecification();
	ImGui::Begin("Compute-First Demo");

	if (!ImGui::CollapsingHeader("Stats")) {
		ImGui::BeginChild("Stats Info", ImVec2(-FLT_MIN, 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);
		ImGui::Text("Mundo: %s", m_Name.c_str());
		ImGui::Text("Carga: %.2f s (en segundo plano, %u hilos)", m_LoadSeconds, Lore::JobSystem::GetWorkerCount());
		ImGui::Text("Resolution: %dx%d", renderer.GetWidth(), renderer.GetHeight());
		ImGui::Text("Framebuffer Size: %dx%d", spec.Width, spec.Height);
		ImGui::Text("FPS: %.2f (%.2f ms)", m_FPS, m_FrameTime * 1000.0f);
		// Fracción de los píxeles reales del framebuffer que traza el compute shader. En
		// pantallas Retina/HiDPI 1.0 son el doble de píxeles por eje que la ventana en
		// puntos; bajarlo (p. ej. 0.5) recupera FPS a cambio de nitidez.
		float renderScale = renderer.GetRenderScale();
		if (ImGui::SliderFloat("Render scale", &renderScale, 0.25f, 1.0f, "%.2f"))
			renderer.SetRenderScale(renderScale);
		ImGui::EndChild();
	}

	if (!ImGui::CollapsingHeader("Maze")) {
		ImGui::BeginChild("Maze Info", ImVec2(-FLT_MIN, 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);
		ImGui::SeparatorText("Maze Generation");

		// Misma semilla = mismo laberinto en cualquier plataforma (Maze::Random).
		ImGui::InputScalar("Seed", ImGuiDataType_U32, &session.MazeSeed);
		ImGui::SameLine();
		if (ImGui::Button("Nueva semilla"))
			session.MazeSeed = static_cast<uint32_t>(std::random_device{}());

		// El laberinto 2D sólo se muestra como texto y se genera al instante: no pasa por
		// el SceneManager.
		ImGui::InputScalar("Grid Dimension", ImGuiDataType_U32, &session.GridDimension);
		session.GridDimension = std::clamp(session.GridDimension, 1u, 200u);

		if (ImGui::BeginTable("Maze2DButtonsTable", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders)) {
			auto button2D = [&](const char* label, Maze::Grid(*algorithm)(Maze::Grid, Maze::Random&)) {
				ImGui::TableNextColumn();
				if (ImGui::Button(label)) {
					Maze::Random rng(session.MazeSeed);
					Maze::Grid grid = algorithm(Maze::Grid(session.GridDimension, session.GridDimension), rng);
					session.GridString = grid.ToString();
				}
			};
			button2D("Binary Tree", &Maze::BinaryTree::On);
			button2D("Wilson's", &Maze::Wilsons::On);
			button2D("Sidewinder", &Maze::Sidewinder::On);
			ImGui::EndTable();
		}
		ScrollingText("Show Maze", session.GridString, 20);

		ImGui::InputScalar("Grid 3D size", ImGuiDataType_U32, &session.Grid3DDimension);
		// CastToVoxels usa 12 vóxeles por celda: 42 celdas por lado ya es un grid denso de
		// 512^3 (512 MB en CPU mientras se construye el SVO). Más que eso no cabe en memoria.
		session.Grid3DDimension = std::clamp(session.Grid3DDimension, 1u, 42u);

		if (ImGui::BeginTable("Maze3DButtonsTable", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders)) {
			auto button3D = [&](const char* label, WorldRecipe::Algorithm algorithm) {
				ImGui::TableNextColumn();
				if (ImGui::Button(label)) {
					WorldRecipe recipe;
					recipe.Type = WorldRecipe::Kind::Maze3D;
					recipe.MazeAlgorithm = algorithm;
					recipe.MazeSize = session.Grid3DDimension;
					recipe.Seed = session.MazeSeed;
					Regenerate(recipe);
				}
			};
			button3D("Binary Tree 3D", WorldRecipe::Algorithm::BinaryTree);
			button3D("Sidewinder 3D", WorldRecipe::Algorithm::Sidewinder);
			button3D("Wilson's 3D", WorldRecipe::Algorithm::Wilsons);
			ImGui::EndTable();
		}
		ScrollingText("Show Maze 3D", m_Grid3DString, 15);

		ImGui::EndChild();
	}

	if (!ImGui::CollapsingHeader("Camera")) {
		ImGui::BeginChild("Camera Info", ImVec2(-FLT_MIN, 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);
		ImGui::SliderFloat3("Camera Pos", camera.GetPositionPtr(), -100.0f, 100.0f, "%.3f");
		ImGui::SliderFloat("Movement Speed", camera.GetMovemetSpeedPtr(), 0.0f, 20.0f, "%.3f");
		ImGui::SliderFloat("Camera angle", camera.GetFovPtr(), 0.0f, 90.0f, "%.3f");
		ImGui::EndChild();
	}

	if (!ImGui::CollapsingHeader("LOD")) {
		Lore::VoxelRenderSettings& settings = session.RenderSettings;
		ImGui::BeginChild("LOD Info", ImVec2(-FLT_MIN, 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);
		ImGui::TextWrapped("El ray marcher deja de bajar por el octree cuando un nodo ya se ve mas chico en pantalla que este umbral, y lo dibuja como un bloque con el color promedio de su subarbol en vez de resolver el detalle exacto. Mas lejos = nodos mas grandes cortados antes = menos resolucion visible; mas cerca = se sigue bajando hasta el detalle fino.");
		ImGui::SliderFloat("Umbral LOD (tamano/distancia)", &settings.LodPixelThreshold, 0.0f, 0.2f, "%.4f");
		ImGui::TextWrapped("0 desactiva el LOD (siempre se baja hasta el fondo del octree).");
		ImGui::SliderFloat("Cobertura minima solida", &settings.LodCoverageThreshold, 0.0f, 1.0f, "%.2f");
		ImGui::TextWrapped("Que tan solido debe ser un nodo lejano para dibujarse opaco en vez de saltarse como aire. Bajarlo hace que estructuras finas/delgadas se sigan distinguiendo de lejos, a costa de infladas.");
		ImGui::EndChild();
	}

	if (!ImGui::CollapsingHeader("Voxel Grid")) {
		ImGui::BeginChild("Voxel Grid Info", ImVec2(-FLT_MIN, 0.0f), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);

		ImGui::SeparatorText("Estalactita de alta resolucion");
		ImGui::InputScalar("Resolucion", ImGuiDataType_U32, &session.StalactiteResolution);
		session.StalactiteResolution = std::clamp(session.StalactiteResolution, 8u, 512u);
		ImGui::TextWrapped("Se redondea a la potencia de 2 mas cercana hacia arriba. Todo el indice se dedica a un unico objeto (en vez de repartirse sobre un mundo grande), asi el mismo costo de memoria produce voxeles mucho mas finos.");
		if (ImGui::Button("Generar Estalactita")) {
			WorldRecipe recipe;
			recipe.Type = WorldRecipe::Kind::Stalactite;
			recipe.Resolution = session.StalactiteResolution;
			Regenerate(recipe);
		}

		ImGui::Spacing();
		ImGui::SeparatorText("Estalagmita de alta resolucion");
		ImGui::InputScalar("Resolucion##Stalagmite", ImGuiDataType_U32, &session.StalagmiteResolution);
		session.StalagmiteResolution = std::clamp(session.StalagmiteResolution, 8u, 512u);
		ImGui::TextWrapped("Mismos principios que la estalactita (mismo generador, invertido): crece del piso hacia arriba en vez de colgar del techo.");
		if (ImGui::Button("Generar Estalagmita")) {
			WorldRecipe recipe;
			recipe.Type = WorldRecipe::Kind::Stalagmite;
			recipe.Resolution = session.StalagmiteResolution;
			Regenerate(recipe);
		}

		ImGui::Spacing();
		if (ImGui::Button("Esfera de prueba"))
			Regenerate(WorldRecipe{});

		double svoMegabytes = (renderer.GetNodeCount() * sizeof(SVO::SVONode)) / (1024.0 * 1024.0);
		ImGui::Text("SVO: %zu nodos (%.2f MB en GPU)", renderer.GetNodeCount(), svoMegabytes);

		ImGui::Separator();
		ImGui::Text("%s", m_VoxelGridString.c_str());

		ImGui::EndChild();
	}

	ImGui::End();
}
