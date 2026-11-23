#include "sndbxpch.h"

#include <Lore.h>
#include <Lore/EntryPoint.h>

#include "Scenes/MazeScene.h"

class Sandbox : public Lore::Application {
public:
	Sandbox() {
		auto session = std::make_shared<SandboxSession>();
		session->Scenes = new Lore::SceneManager();
		PushLayer(session->Scenes);

		// Arranca con la escena de prueba (esfera); los botones de la UI piden las demás.
		session->Scenes->LoadAsync(std::make_unique<MazeScene>(session, WorldRecipe{}));
	}
	~Sandbox() {}

};

Lore::Application* Lore::CreateApplication() {
	return new Sandbox();
}
