#include <Lore.h>
#include <Lore/EntryPoint.h>

#include "Game/GameLayer.h"

class Sandbox : public Lore::Application {
public:
	Sandbox() {
		PushLayer(new GameLayer());
	}
	~Sandbox() {}

};

Lore::Application* Lore::CreateApplication() {
	return new Sandbox();
}
