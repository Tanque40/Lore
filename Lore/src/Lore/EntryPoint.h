#pragma once

#include "Lore/Application.h"
#include "Lore/Log.h"

extern Lore::Application* Lore::CreateApplication();

int main(int argc, const char** argv) {

	Lore::Log::Init();
	LR_CORE_WARN("Initialize log...");
	int n = 6;
	LR_INFO("Hello! The var is: {0}", n);

	std::cout << "Lore Engine Initialized!" << std::endl;
	auto app = Lore::CreateApplication();
	app->Run();
	delete app;

	return 0;
}
