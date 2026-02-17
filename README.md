# Lore Engine

**Lore Engine** es un motor de juegos cross-platform enfocado en manipulación voxel, construido con C++ moderno y OpenGL 4.1.

El proyecto está en una **fase de fundación temprana**: el sistema de ventanas, eventos, input, capas e ImGui están funcionales. El sistema de renderizado es el siguiente módulo principal a construir.

---

## Características Implementadas

- **Soporte Cross-Platform**: Windows (x64) y macOS (ARM64)
- **Arquitectura por Capas**: Sistema modular de `Layer`/`LayerStack` para organizar lógica de juego y renderizado
- **Sistema de Eventos**: Despacho inmediato con tipado seguro y categorías (Application, Input, Keyboard, Mouse)
- **Integración ImGui**: Overlay de Dear ImGui con soporte de docking y viewports múltiples
- **Sistema de Logging**: Logger dual (Core/Client) sobre spdlog con formato coloreado
- **OpenGL Moderno**: Contexto OpenGL 4.1 con GLAD loader y ventanas GLFW
- **Input Polling**: Consulta de estado de teclado y ratón en cualquier momento del frame

---

## Arquitectura

### Diagrama General

```mermaid
graph TB
    subgraph Sandbox["Sandbox (ConsoleApp)"]
        SandboxApp["Sandbox : Application"]
        ExampleLayer["ExampleLayer : Layer"]
        SandboxApp -->|PushLayer| ExampleLayer
    end

    subgraph LoreEngine["Lore Engine (StaticLib)"]

        subgraph Core["Core"]
            Application["Application<br/><i>Singleton, game loop</i>"]
            EntryPoint["EntryPoint.h<br/><i>define main()</i>"]
            Log["Log<br/><i>spdlog wrapper</i>"]
            CoreH["Core.h<br/><i>macros, asserts</i>"]
        end

        subgraph LayerSystem["Layer System"]
            LayerStack["LayerStack<br/><i>contenedor ordenado</i>"]
            Layer["Layer<br/><i>base class</i>"]
            ImGuiLayer["ImGuiLayer : Layer<br/><i>overlay de debug</i>"]
            LayerStack -->|contiene| Layer
            LayerStack -->|contiene| ImGuiLayer
        end

        subgraph Events["Event System"]
            Event["Event<br/><i>abstract base</i>"]
            EventDispatcher["EventDispatcher<br/><i>template dispatch</i>"]
            WindowEvents["WindowClose<br/>WindowResize"]
            KeyEvents["KeyPressed<br/>KeyReleased<br/>KeyTyped"]
            MouseEvents["MouseMoved<br/>MouseScrolled<br/>MouseButton*"]
            Event --- WindowEvents
            Event --- KeyEvents
            Event --- MouseEvents
        end

        subgraph InputSystem["Input"]
            Input["Input<br/><i>singleton virtual</i>"]
            KeyCodes["KeyCodes.h<br/><i>120+ macros</i>"]
            MouseCodes["MouseButtonCodes.h"]
        end

        subgraph Platform["Platform Layer"]
            subgraph WinPlatform["Windows"]
                WindowsWindow["WindowsWindow : Window"]
                WindowsInput["WindowsInput : Input"]
            end
            subgraph MacPlatform["macOS"]
                MacWindow["MacWindow : Window"]
                MacInput["MacInput : Input"]
            end
            subgraph OpenGLPlatform["OpenGL"]
                OpenGLContext["OpenGLContext : GraphicsContext"]
            end
        end

        subgraph Renderer["Renderer"]
            Window["Window<br/><i>interfaz pura</i>"]
            GraphicsContext["GraphicsContext<br/><i>interfaz pura</i>"]
        end
    end

    SandboxApp -.->|extends| Application
    Application --> LayerStack
    Application --> Window
    Application --> Log
    Application --> ImGuiLayer
    Window -.->|implementa| WindowsWindow
    Window -.->|implementa| MacWindow
    GraphicsContext -.->|implementa| OpenGLContext
    Input -.->|implementa| WindowsInput
    Input -.->|implementa| MacInput
    WindowsWindow --> OpenGLContext
    MacWindow --> OpenGLContext
    EntryPoint -->|crea| Application

    subgraph Vendors["Dependencias (vendor/)"]
        GLFW["GLFW<br/><i>ventanas + input</i>"]
        GLAD["GLAD<br/><i>OpenGL loader</i>"]
        IMGUI["Dear ImGui<br/><i>GUI inmediata</i>"]
        spdlog["spdlog<br/><i>logging</i>"]
        glm["glm<br/><i>math (sin usar aún)</i>"]
    end

    WindowsWindow --> GLFW
    MacWindow --> GLFW
    OpenGLContext --> GLAD
    ImGuiLayer --> IMGUI
    Log --> spdlog
```

### Diagrama de Clases

```mermaid
classDiagram
    class Application {
        -unique_ptr~Window~ m_Window
        -ImGuiLayer* m_ImGuiLayer
        -bool m_Running
        -LayerStack m_LayerStack
        -static Application* s_Instance
        -unsigned int m_VertexArray
        -unsigned int m_VertexBuffer
        -unsigned int m_IndexBuffer
        +Run() void
        +OnEvent(Event&) void
        +PushLayer(Layer*) void
        +PushOverlay(Layer*) void
        +static Get() Application&
        +GetWindow() Window&
    }

    class Window {
        <<interface>>
        +OnUpdate()* void
        +GetWidth()* uint
        +GetHeight()* uint
        +SetEventCallback(fn)* void
        +SetVSync(bool)* void
        +IsVSync()* bool
        +GetNativeWindow()* void*
        +static Create(WindowProps) Window*
    }

    class GraphicsContext {
        <<interface>>
        +Init()* void
        +SwapBuffers()* void
    }

    class Input {
        <<abstract>>
        -static Input* s_Instance
        +static IsKeyPressed(int) bool
        +static IsMouseButtonPressed(int) bool
        +static GetMousePosition() pair
        #IsKeyPressedImpl(int)* bool
        #IsMouseButtonPressedImpl(int)* bool
        #GetMousePositionImpl()* pair
    }

    class Layer {
        -string m_DebugName
        +OnAttach() void
        +OnDetach() void
        +OnUpdate() void
        +OnImGuiRender() void
        +OnEvent(Event&) void
        +GetName() string
    }

    class LayerStack {
        -vector~Layer*~ m_Layers
        -unsigned int m_LayerInsertIndex
        +PushLayer(Layer*) void
        +PushOverlay(Layer*) void
        +PopLayer(Layer*) void
        +PopOverlay(Layer*) void
        +begin() iterator
        +end() iterator
    }

    class Event {
        <<abstract>>
        +bool m_Handled
        +GetEventType()* EventType
        +GetName()* string
        +GetCategoryFlags()* int
        +IsInCategory(EventCategory) bool
        +ToString() string
    }

    class Log {
        -static shared_ptr s_CoreLogger
        -static shared_ptr s_ClientLogger
        +static Init() void
        +static GetCoreLogger()
        +static GetClientLogger()
    }

    class ImGuiLayer {
        +OnAttach() void
        +OnDetach() void
        +OnImGuiRender() void
        +Begin() void
        +End() void
    }

    class WindowsWindow {
        -GLFWwindow* m_Window
        -GraphicsContext* m_Context
        -WindowData m_Data
    }
    class MacWindow {
        -GLFWwindow* m_Window
        -GraphicsContext* m_Context
        -WindowData m_Data
    }
    class OpenGLContext {
        -GLFWwindow* m_WindowHandle
        +Init() void
        +SwapBuffers() void
    }
    class WindowsInput
    class MacInput

    Application *-- LayerStack
    Application *-- Window
    Application --> ImGuiLayer
    Application --> Log

    LayerStack o-- Layer
    ImGuiLayer --|> Layer

    Window <|.. WindowsWindow
    Window <|.. MacWindow
    GraphicsContext <|.. OpenGLContext
    Input <|.. WindowsInput
    Input <|.. MacInput

    WindowsWindow --> OpenGLContext
    MacWindow --> OpenGLContext

    Event <|-- KeyEvent
    Event <|-- MouseMovedEvent
    Event <|-- MouseScrolledEvent
    Event <|-- MouseButtonEvent
    Event <|-- WindowResizeEvent
    Event <|-- WindowCloseEvent
    KeyEvent <|-- KeyPressedEvent
    KeyEvent <|-- KeyReleasedEvent
    KeyEvent <|-- KeyTypedEvent
    MouseButtonEvent <|-- MouseButtonPressedEvent
    MouseButtonEvent <|-- MouseButtonReleasedEvent
```

### Flujo del Game Loop

```mermaid
sequenceDiagram
    participant EP as EntryPoint (main)
    participant App as Application
    participant Win as Window (GLFW)
    participant LS as LayerStack
    participant IG as ImGuiLayer
    participant GL as OpenGLContext

    EP->>App: CreateApplication()
    EP->>App: Run()

    loop Game Loop (while m_Running)
        App->>Win: glClear (pantalla)
        App->>App: glDrawElements (triángulo test)

        loop Para cada Layer
            App->>LS: layer->OnUpdate()
        end

        App->>IG: Begin()
        loop Para cada Layer
            App->>LS: layer->OnImGuiRender()
        end
        App->>IG: End()

        App->>Win: OnUpdate()
        Win->>Win: glfwPollEvents()
        Win->>GL: SwapBuffers()
    end
```

### Flujo de Eventos

```mermaid
sequenceDiagram
    participant GLFW
    participant Win as Window
    participant App as Application
    participant Disp as EventDispatcher
    participant LS as LayerStack
    participant L as Layer (último → primero)

    GLFW->>Win: Callback (key/mouse/window)
    Win->>Win: Crea Event concreto
    Win->>App: EventCallback(event)
    App->>Disp: Dispatch<WindowCloseEvent>
    alt WindowCloseEvent
        Disp->>App: m_Running = false
    end
    loop Capas en orden inverso
        App->>L: OnEvent(event)
        alt event.m_Handled
            Note over L: Se detiene la propagación
        end
    end
```

---

## Estructura del Proyecto

```
Lore/                          # Raíz del workspace
├── premake5.lua               # Build system principal (Premake5)
├── Lore.slnx                  # Solución Visual Studio
├── WindowsSetupPremake.bat    # Descarga Premake para Windows
├── WindowsSetupProject.bat    # Genera proyecto Visual Studio
│
├── Lore/                      # Motor (StaticLib)
│   ├── src/
│   │   ├── Lore.h             # Header público único (include todo)
│   │   ├── lrpch.h / .cpp     # Precompiled header
│   │   └── Lore/
│   │       ├── Application.h/.cpp    # Singleton, game loop, ventana
│   │       ├── Core.h               # Macros: plataforma, asserts, BIT()
│   │       ├── EntryPoint.h          # Define main()
│   │       ├── Input.h              # Singleton abstracto de input
│   │       ├── KeyCodes.h           # 120+ macros LR_KEY_*
│   │       ├── MouseButtonCodes.h   # 8 botones de ratón
│   │       ├── Layer.h/.cpp         # Clase base Layer
│   │       ├── LayerStack.h/.cpp    # Contenedor ordenado de capas
│   │       ├── Log.h/.cpp           # Wrapper spdlog (Core + Client)
│   │       ├── Window.h             # Interfaz pura de ventana
│   │       │
│   │       ├── Events/              # Sistema de eventos
│   │       │   ├── Event.h          # Base abstracta + EventDispatcher
│   │       │   ├── ApplicationEvent.h  # WindowClose, WindowResize, AppTick...
│   │       │   ├── KeyEvent.h       # KeyPressed, KeyReleased, KeyTyped
│   │       │   └── MouseEvent.h     # MouseMoved, MouseScrolled, MouseButton*
│   │       │
│   │       ├── ImGui/               # Integración Dear ImGui
│   │       │   ├── ImGuiLayer.h/.cpp   # Capa overlay (docking + viewports)
│   │       │   ├── ImGuiBuild.cpp      # Unity build de backends ImGui
│   │       │   └── ImGuiKeyCodes.h     # Mapeo LR_KEY → ImGuiKey (sin usar)
│   │       │
│   │       ├── Platform/            # Implementaciones por plataforma
│   │       │   ├── Windows/
│   │       │   │   ├── WindowsWindow.h/.cpp  # GLFW window para Windows
│   │       │   │   └── WindowsInput.h/.cpp   # Input polling para Windows
│   │       │   ├── Mac/
│   │       │   │   ├── MacWindow.h/.cpp      # GLFW window para macOS
│   │       │   │   └── MacInput.h/.cpp       # Input polling para macOS
│   │       │   └── OpenGL/
│   │       │       └── OpenGLContext.h/.cpp   # Init GLAD + swap buffers
│   │       │
│   │       └── Renderer/            # (En desarrollo)
│   │           └── GraphicsContext.h  # Interfaz pura (Init + SwapBuffers)
│   │
│   └── vendor/                # Dependencias del motor
│       ├── GLFW/              # Windowing y input nativo
│       ├── GLAD/              # OpenGL function loader
│       ├── IMGUI/             # Dear ImGui (rama docking)
│       ├── glm/               # Matemáticas (incluido, sin usar aún)
│       └── spdlog/            # Logging rápido (header-only)
│
├── Sandbox/                   # Aplicación de ejemplo (ConsoleApp)
│   └── src/
│       └── SandboxApp.cpp     # ExampleLayer + CreateApplication()
│
├── Scripts/                   # Scripts de build
│   └── Macos/
│       ├── Helpers/           # RunDebugSandboxProject, RunPremake
│       └── Main/              # Bootstrap.sh
│
├── Docs/                      # Documentación
│   └── Diagrams/
│       └── Architecture.plantuml
│
└── vendor/                    # Herramientas de build
    └── premake/               # Binarios de Premake5
```

---

## Jerarquía de Clases

```
GraphicsContext (interfaz)
└── OpenGLContext

Window (interfaz + factory)
├── WindowsWindow
└── MacWindow

Input (singleton + virtual dispatch)
├── WindowsInput
└── MacInput

Event (abstracta)
├── WindowResizeEvent
├── WindowCloseEvent
├── AppTickEvent / AppUpdateEvent / AppRenderEvent
├── KeyEvent (abstracta)
│   ├── KeyPressedEvent
│   ├── KeyReleasedEvent
│   └── KeyTypedEvent
├── MouseMovedEvent
├── MouseScrolledEvent
└── MouseButtonEvent (abstracta)
    ├── MouseButtonPressedEvent
    └── MouseButtonReleasedEvent

Layer (base, todos los métodos virtuales son no-op)
└── ImGuiLayer

Application (singleton, el cliente hereda de esta)
└── Sandbox (código cliente)
    └── ExampleLayer
```

---

## Dependencias

| Librería                                       | Versión / Rama | Propósito                                                        |
| ---------------------------------------------- | -------------- | ---------------------------------------------------------------- |
| [GLFW](https://www.glfw.org/)                  | —              | Creación de ventanas, contexto OpenGL, input nativo              |
| [GLAD](https://glad.dav1d.de/)                 | OpenGL 4.1     | Loader de funciones OpenGL                                       |
| [Dear ImGui](https://github.com/ocornut/imgui) | Rama docking   | GUI inmediata con docking y multi-viewports                      |
| [spdlog](https://github.com/gabime/spdlog)     | Header-only    | Logging rápido con formato y colores                             |
| [glm](https://github.com/g-truc/glm)           | —              | Matemáticas (vectores, matrices). **Incluido pero sin usar aún** |
| [Premake5](https://premake.github.io/)         | —              | Generación de proyectos (VS, Makefiles)                          |

---

## Estado del Proyecto

### Implementado

| Módulo              | Estado    | Descripción                                                                |
| ------------------- | --------- | -------------------------------------------------------------------------- |
| Windowing (GLFW)    | Completo  | Creación de ventana, VSync, callbacks. Windows + Mac                       |
| Sistema de Eventos  | Completo  | Despacho inmediato por tipo con `EventDispatcher`. 14 tipos de evento      |
| Input Polling       | Completo  | Teclado + ratón, ambas plataformas                                         |
| Logging (spdlog)    | Completo  | Logger dual Core (`LORE`) / Client (`APP`) con macros                      |
| Sistema de Capas    | Completo  | `LayerStack` con capas normales y overlays. Propagación inversa de eventos |
| Integración ImGui   | Completo  | Docking, viewports múltiples, demo window, backends GLFW + OpenGL3         |
| Contexto OpenGL     | Completo  | OpenGL 4.1, GLAD loader, info de GPU al inicio                             |
| Triángulo de prueba | Funcional | VAO/VBO/EBO con llamadas OpenGL directas en `Application.cpp`              |

### En Progreso / Pendiente

| Módulo                        | Estado              | Notas                                                                                                                        |
| ----------------------------- | ------------------- | ---------------------------------------------------------------------------------------------------------------------------- |
| Renderer Abstraction          | **No iniciado**     | Solo existe la interfaz `GraphicsContext`. Faltan: Shader, VertexBuffer, IndexBuffer, VertexArray, RenderCommand, Renderer2D |
| Delta Time                    | **No implementado** | `OnUpdate()` no recibe timestep                                                                                              |
| Event Buffering               | **No implementado** | Los eventos se despachan inmediatamente (se menciona como trabajo futuro en `Event.h`)                                       |
| Eventos WindowFocus/Move      | **No implementado** | Los enum values existen pero no hay callbacks GLFW que los disparen                                                          |
| glm (math)                    | **Sin usar**        | Está vendorizado e incluido en los paths, pero ningún archivo del motor lo referencia                                        |
| Cámara                        | **No iniciado**     | No hay sistema de cámara                                                                                                     |
| Shaders                       | **No iniciado**     | No hay abstracción de shaders                                                                                                |
| Manejo de Texturas            | **No iniciado**     | —                                                                                                                            |
| Scene Graph                   | **No iniciado**     | —                                                                                                                            |
| ECS (Entity Component System) | **No iniciado**     | —                                                                                                                            |
| Audio                         | **No iniciado**     | —                                                                                                                            |
| Física                        | **No iniciado**     | —                                                                                                                            |
| Voxel System                  | **No iniciado**     | Objetivo final del motor                                                                                                     |

### Roadmap Sugerido

```mermaid
gantt
    title Roadmap de Desarrollo - Lore Engine
    dateFormat YYYY-MM
    axisFormat %b %Y

    section Fundación
    Windowing + Input         :done, 2025-01, 2025-04
    Event System              :done, 2025-02, 2025-04
    Layer System              :done, 2025-03, 2025-05
    ImGui Overlay             :done, 2025-04, 2025-06
    Logging                   :done, 2025-01, 2025-03

    section Renderer
    Shader Abstraction        :active, 2026-02, 2026-04
    Buffer Objects (VBO/VAO)  : 2026-03, 2026-05
    Render Commands           : 2026-04, 2026-06
    Cámara 2D/3D              : 2026-05, 2026-07
    Texturas                  : 2026-06, 2026-08
    Renderer2D                : 2026-07, 2026-09

    section Motor de Juego
    Delta Time / Timestep     : 2026-03, 2026-04
    Scene Graph               : 2026-08, 2026-10
    ECS                       : 2026-09, 2026-12
    Sistema Voxel             : 2027-01, 2027-06
```

---

## Building

### Requisitos Previos

- Compilador C++17 o superior
- **macOS**: Xcode Command Line Tools
- **Windows**: Visual Studio 2019+ (recomendado 2022)

### Windows

```batch
:: 1. Descargar Premake5
WindowsSetupPremake.bat

:: 2. Generar solución Visual Studio
WindowsSetupProject.bat
```

Luego abrir `Lore.slnx` en Visual Studio y compilar. El proyecto `Sandbox` está configurado como startup project.

### macOS (ARM64)

```bash
# Bootstrap premake y generar makefiles
./Scripts/Macos/Main/Bootstrap.sh

# Compilar y ejecutar
make config=debug_macosx-aarch64
./bin/Debug-macosx-AARCH64/Sandbox/Sandbox
```

---

## Uso

### Crear una Aplicación

El motor define `main()` internamente mediante `EntryPoint.h`. Solo necesitas definir `CreateApplication()`:

```cpp
#include <Lore.h>

class GameLayer : public Lore::Layer {
public:
    GameLayer() : Layer("Game") {}

    void OnUpdate() override {
        // Lógica de juego
        if (Lore::Input::IsKeyPressed(LR_KEY_W))
            LR_INFO("Tecla W presionada");
    }

    void OnImGuiRender() override {
        ImGui::Begin("Debug");
        ImGui::Text("Hola desde Lore Engine!");
        ImGui::End();
    }

    void OnEvent(Lore::Event& event) override {
        // Manejar eventos específicos
        Lore::EventDispatcher dispatcher(event);
        dispatcher.Dispatch<Lore::KeyPressedEvent>(
            [](Lore::KeyPressedEvent& e) {
                LR_INFO("Key: {0}", e.GetKeyCode());
                return false;
            });
    }
};

class MyGame : public Lore::Application {
public:
    MyGame() {
        PushLayer(new GameLayer());
    }
};

Lore::Application* Lore::CreateApplication() {
    return new MyGame();
}
```

> **Nota**: `ImGuiLayer` se añade automáticamente como overlay en el constructor de `Application`. No es necesario añadirlo manualmente.

### Sistema de Logging

```cpp
// Logger del motor (LORE)
LR_CORE_INFO("Motor inicializado");
LR_CORE_WARN("Advertencia: {0}", mensaje);
LR_CORE_ERROR("Error: código {0}", codigo);
LR_CORE_TRACE("Frame {0}", frameCount);

// Logger del cliente (APP)
LR_INFO("Jugador en ({0}, {1})", x, y);
LR_WARN("Vida baja: {0}", salud);
LR_ERROR("No se pudo cargar: {0}", archivo);
```

### Input Polling

```cpp
// En cualquier momento dentro de OnUpdate()
if (Lore::Input::IsKeyPressed(LR_KEY_SPACE))
    // saltar

if (Lore::Input::IsMouseButtonPressed(LR_MOUSE_BUTTON_LEFT))
    // clic izquierdo

auto [x, y] = Lore::Input::GetMousePosition();
```

---

## Configuraciones de Build

| Config  | Defines                         | Descripción                                      |
| ------- | ------------------------------- | ------------------------------------------------ |
| Debug   | `LR_DEBUG`, `LR_ENABLE_ASSERTS` | Símbolos de debug completos, asserts habilitados |
| Release | `LR_RELEASE`                    | Build optimizado                                 |
| Dist    | `LR_DIST`                       | Distribución, optimización máxima                |

---

## Patrones de Diseño Utilizados

| Patrón              | Uso                                                                     |
| ------------------- | ----------------------------------------------------------------------- |
| **Singleton**       | `Application`, `Input` — acceso global a instancia única                |
| **Factory Method**  | `Window::Create()` — crea la implementación correcta por plataforma     |
| **Observer**        | Sistema de eventos — callbacks GLFW → `Event` → `Application` → `Layer` |
| **Template Method** | `Layer` — define hooks virtuales que las subclases implementan          |
| **Strategy**        | `Input`, `Window` — interfaz común con implementaciones intercambiables |
| **Composite**       | `LayerStack` — colección ordenada de `Layer` con iteración uniforme     |

---

## Licencia

Este proyecto está bajo la Licencia MIT — ver el archivo [LICENSE](LICENSE) para más detalles.

Copyright (c) 2026 Bruno Vitte
