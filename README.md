# Lore Engine

**Lore Engine** is a cross-platform game engine centered on voxel manipulation, built with modern C++ and OpenGL.

## Features

- **Cross-Platform Support**: Windows and macOS (ARM64)
- **Layer-Based Architecture**: Modular layer system for organizing game logic and rendering
- **Event System**: Type-safe event dispatching with categories for Application, Input, Keyboard, Mouse events
- **ImGui Integration**: Built-in Dear ImGui overlay for debugging and UI
- **Logging System**: Dual-logger system (Core/Client) powered by spdlog
- **Modern OpenGL**: GLAD loader with GLFW windowing

## Architecture

```
Lore/
├── src/
│   ├── Lore/
│   │   ├── Application      # Main application loop and window management
│   │   ├── Layer/LayerStack # Layer system for game logic organization
│   │   ├── Events/          # Event system (Window, Key, Mouse, Application)
│   │   ├── ImGui/           # ImGui integration layer
│   │   └── Platform/        # Platform-specific implementations (Mac, Windows)
│   └── Lore.h               # Single include header for client applications
├── vendor/                  # Third-party libraries
│   ├── GLFW/                # Windowing and input
│   ├── GLAD/                # OpenGL loader
│   ├── IMGUI/               # Dear ImGui
│   └── spdlog/              # Fast logging library
Sandbox/                     # Example application
```

## Dependencies

| Library                                        | Purpose                            |
| ---------------------------------------------- | ---------------------------------- |
| [GLFW](https://www.glfw.org/)                  | Window creation and input handling |
| [GLAD](https://glad.dav1d.de/)                 | OpenGL function loader             |
| [Dear ImGui](https://github.com/ocornut/imgui) | Immediate mode GUI                 |
| [spdlog](https://github.com/gabime/spdlog)     | Fast C++ logging                   |
| [Premake5](https://premake.github.io/)         | Build configuration                |

## Building

### Prerequisites

- C++17 or later compiler
- Make
- macOS: Xcode Command Line Tools
- Windows: Visual Studio 2019+ or MinGW

### macOS (ARM64)

```bash
# Bootstrap premake and generate makefiles
./Scripts/Macos/Main/Bootstrap.sh

# Build and run
make
./bin/Debug-macosx-AARCH64/Sandbox/Sandbox
```

### Windows

```batch
# Generate Visual Studio solution
WindowsSetupPremake.bat
WindowsSetupProject.bat
```

Then open the generated `.sln` file in Visual Studio and build.

## Usage

Create your own application by inheriting from `Lore::Application`:

```cpp
#include <Lore.h>

class ExampleLayer : public Lore::Layer {
public:
    ExampleLayer() : Layer("Example") {}

    void OnUpdate() override {
        // Game logic here
    }

    void OnEvent(Lore::Event& event) override {
        LR_INFO("{0}", event.ToString());
    }
};

class MyGame : public Lore::Application {
public:
    MyGame() {
        PushLayer(new ExampleLayer());
        PushOverlay(new Lore::ImGuiLayer());
    }
};

Lore::Application* Lore::CreateApplication() {
    return new MyGame();
}
```

### Logging

```cpp
// Core logging (engine internals)
LR_CORE_INFO("Engine initialized");
LR_CORE_WARN("Warning message");
LR_CORE_ERROR("Error occurred");

// Client logging (game code)
LR_INFO("Player spawned at {0}, {1}", x, y);
LR_WARN("Low health: {0}", health);
```

## Build Configurations

| Configuration | Description                                |
| ------------- | ------------------------------------------ |
| Debug         | Full debugging symbols, assertions enabled |
| Release       | Optimized build                            |
| Dist          | Distribution build, fully optimized        |

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

Copyright (c) 2026 Bruno Vitte
