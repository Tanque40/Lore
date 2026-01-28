#include <Lore.h>

class Sandbox : public Lore::Application {

public:
	Sandbox() {}
	~Sandbox() {}

};


Lore::Application* Lore::CreateApplication() {
	return new Sandbox();
}


