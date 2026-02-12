#include <Lore.h>

class ExampleLayer : public Lore::Layer {
public:
	ExampleLayer() : Layer("Example") {}

	void OnUpdate() override {
		LR_INFO("ExampleLayer::Update");
	}

	void OnEvent(Lore::Event& event) override {
		LR_INFO("{0}", event.ToString());
	}
};

class Sandbox : public Lore::Application {

public:
	Sandbox() {
		PushLayer(new ExampleLayer());
		PushOverlay(new Lore::ImGuiLayer());
	}
	~Sandbox() {}

};


Lore::Application* Lore::CreateApplication() {
	return new Sandbox();
}


