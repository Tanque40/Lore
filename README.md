# Lore Engine

**Lore Engine** es un motor de juegos cross-platform enfocado en manipulación voxel y raytracing mediante Sparse Voxel Octrees (SVO), construido con C++23 y un sistema de renderizado multi-API (OpenGL 4.1 en Windows, Apple Metal en macOS).

El proyecto cuenta con un **pipeline compute-first completo** incluyendo compute shaders, framebuffers offscreen, cámara FPS en perspectiva, un sistema SVO funcional, generación procedural de laberintos 2D/3D, un compositor dockeable con ImGui, y selección automática de API gráfica por plataforma.

---

## Características Implementadas

- **Soporte Cross-Platform**: Windows (x64) con OpenGL 4.1 y macOS (ARM64) con Apple Metal
- **Multi-API Rendering**: Selección automática de backend gráfico en tiempo de compilación (OpenGL / Metal)
- **Compute Shader Pipeline**: Dispatch de compute shaders con texturas de escritura/lectura y blit a pantalla completa
- **Framebuffer Offscreen**: Renderizado a textura con resize dinámico e integración en viewport ImGui
- **Renderer Abstraction**: Sistema completo con Shader, VertexBuffer, IndexBuffer, VertexArray, StorageBuffer, RenderCommand y Renderer
- **Cámara Ortográfica**: OrthographicCamera con posición, rotación y matrices View/Projection
- **Cámara en Perspectiva (FPS)**: PerspectiveCamera con movimiento WASD, Yaw/Pitch por ratón, FOV ajustable y velocidad configurable
- **Delta Time (TimeStep)**: Clase `TimeStep` con delta time por frame, pasado a `Layer::OnUpdate()`
- **Sparse Voxel Octree (SVO)**: VoxelGrid (256³), SVOBuilder recursivo, empaquetado GPU-ready de 8 bytes por nodo
- **Generación de Laberintos 2D/3D**: Grid/Grid3D con algoritmos BinaryTree, Wilson's y Sidewinder
- **StorageBuffer**: Buffer de almacenamiento GPU (SSBO en OpenGL, MTLBuffer en Metal) para datos arbitrarios
- **Control de Cursor**: `HideCursor()`/`ShowCursor()` para modo FPS con captura de ratón
- **Matemáticas con glm**: Vectores, matrices y transformaciones integradas
- **Arquitectura por Capas**: Sistema modular de `Layer`/`LayerStack` para organizar lógica de juego y renderizado
- **Sistema de Eventos**: Despacho inmediato con tipado seguro y categorías (Application, Input, Keyboard, Mouse)
- **ImGui con Dockspace**: Overlay de Dear ImGui con dockspace, auto-layout, viewport de framebuffer y soporte multi-viewports
- **Sistema de Logging**: Logger dual (Core/Client) sobre spdlog con formato coloreado
- **Input Polling**: Consulta de estado de teclado y ratón en cualquier momento del frame
- **Soporte Retina/HiDPI**: `GetFramebufferWidth()`/`GetFramebufferHeight()` para resoluciones de display escaladas

---

## Arquitectura

### Diagrama General

```mermaid
graph TB
    subgraph Sandbox["Sandbox (ConsoleApp)"]
        SandboxApp["Sandbox : Application"]
        GameLayerNode["GameLayer : Layer"]
        SandboxApp -->|PushLayer| GameLayerNode

        subgraph ComputeDemo["Compute-First Pipeline"]
            CompShader["ComputeShader"]
            CompTexture["ComputeTexture"]
            SBuf["StorageBuffer<br/><i>SVO data → GPU</i>"]
            CompShader --> CompTexture
            CompShader --> SBuf
        end
        GameLayerNode --> ComputeDemo

        subgraph CameraModule["Camera"]
            PerspCam["PerspectiveCamera<br/><i>FPS: WASD + Mouse</i>"]
        end
        GameLayerNode --> CameraModule

        subgraph MazeModule["Maze Module"]
            MazeGrid["Grid<br/><i>matriz 2D de celdas</i>"]
            MazeGrid3D["Grid3D<br/><i>matriz 3D de celdas</i>"]
            MazeCell["Cell<br/><i>conexiones N/S/E/W</i>"]
            MazeCell3D["Cell3D<br/><i>Cell + Up/Down</i>"]
            BinaryTree["BinaryTree<br/><i>2D y 3D</i>"]
            Wilsons["Wilsons<br/><i>random walk sin loops</i>"]
            Sidewinder["Sidewinder<br/><i>generación por filas</i>"]
            MazeGrid -->|contiene| MazeCell
            MazeGrid3D -->|contiene| MazeCell3D
            MazeCell3D -->|hereda| MazeCell
            BinaryTree -->|genera| MazeGrid
            BinaryTree -->|genera| MazeGrid3D
            Wilsons -->|genera| MazeGrid
            Sidewinder -->|genera| MazeGrid
        end
        GameLayerNode --> MazeModule

        subgraph SVOModule["SVO Module"]
            VoxelGridNode["VoxelGrid<br/><i>256³ grid volumétrico</i>"]
            SVOBuilderNode["SVOBuilder<br/><i>construcción recursiva</i>"]
            SVONodeStruct["SVONode<br/><i>8 bytes GPU-packed</i>"]
            VoxelGridNode -->|input| SVOBuilderNode
            SVOBuilderNode -->|genera| SVONodeStruct
        end
        GameLayerNode --> SVOModule
    end

    subgraph LoreEngine["Lore Engine (StaticLib)"]

        subgraph Core["Core"]
            Application["Application<br/><i>Singleton, game loop</i>"]
            EntryPoint["EntryPoint.h<br/><i>define main()</i>"]
            Log["Log<br/><i>spdlog wrapper</i>"]
            CoreH["Core.h<br/><i>macros, asserts</i>"]
            TimeStepClass["TimeStep<br/><i>delta time</i>"]
        end

        subgraph LayerSystem["Layer System"]
            LayerStack["LayerStack<br/><i>contenedor ordenado</i>"]
            Layer["Layer<br/><i>base class</i>"]
            ImGuiLayer["ImGuiLayer : Layer<br/><i>dockspace + viewport</i>"]
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
            subgraph OpenGLPlatform["OpenGL Backend"]
                OpenGLContext["OpenGLContext"]
                OpenGLRendererAPI["OpenGLRendererAPI"]
                OpenGLShader["OpenGLShader"]
                OpenGLBuffer["OpenGLBuffer"]
                OpenGLVA["OpenGLVertexArray"]
                OpenGLFB["OpenGLFramebuffer"]
                OpenGLCS["OpenGLComputeShader"]
                OpenGLCT["OpenGLComputeTexture"]
            end
            subgraph MetalPlatform["Metal Backend"]
                MetalContext["MetalContext"]
                MetalRendererAPI["MetalRendererAPI"]
                MetalShader["MetalShader"]
                MetalBuffer["MetalBuffer"]
                MetalVA["MetalVertexArray"]
                MetalFB["MetalFramebuffer"]
                MetalCS["MetalComputeShader"]
                MetalCT["MetalComputeTexture"]
            end
        end

        subgraph Renderer["Renderer"]
            Window["Window<br/><i>interfaz pura</i>"]
            GraphicsContext["GraphicsContext<br/><i>interfaz pura</i>"]
            RendererClass["Renderer<br/><i>BeginScene/Submit/BlitToScreen</i>"]
            RenderCommand["RenderCommand<br/><i>Draw/Compute/Blit</i>"]
            ShaderClass["Shader<br/><i>uniforms, bind</i>"]
            BufferClasses["VertexBuffer / IndexBuffer / StorageBuffer"]
            VertexArrayClass["VertexArray"]
            FramebufferClass["Framebuffer<br/><i>offscreen rendering</i>"]
            ComputeShaderClass["ComputeShader<br/><i>GPGPU dispatch</i>"]
            ComputeTextureClass["ComputeTexture<br/><i>image read/write</i>"]
            CameraClass["OrthographicCamera<br/><i>View/Projection matrices</i>"]
            PerspCameraClass["PerspectiveCamera<br/><i>FPS: Yaw/Pitch/FOV</i>"]
        end
    end

    SandboxApp -.->|extends| Application
    Application --> LayerStack
    Application --> Window
    Application --> Log
    Application --> ImGuiLayer
    Application --> FramebufferClass
    Application --> TimeStepClass
    Window -.->|implementa| WindowsWindow
    Window -.->|implementa| MacWindow
    GraphicsContext -.->|implementa| OpenGLContext
    GraphicsContext -.->|implementa| MetalContext
    Input -.->|implementa| WindowsInput
    Input -.->|implementa| MacInput
    WindowsWindow --> OpenGLContext
    MacWindow --> MetalContext
    EntryPoint -->|crea| Application

    subgraph Vendors["Dependencias (vendor/)"]
        GLFW["GLFW<br/><i>ventanas + input</i>"]
        GLAD["GLAD<br/><i>OpenGL loader</i>"]
        IMGUI["Dear ImGui<br/><i>GUI inmediata</i>"]
        spdlog["spdlog<br/><i>logging</i>"]
        glm["glm<br/><i>matrices, vectores</i>"]
        MetalFW["Metal.framework<br/><i>GPU API (macOS)</i>"]
    end

    WindowsWindow --> GLFW
    MacWindow --> GLFW
    OpenGLContext --> GLAD
    MetalContext --> MetalFW
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
        -Framebuffer* m_Framebuffer
        -TimeStep m_TimeStep
        -float m_LastFrameTime
        -static Application* s_Instance
        +Run() void
        +OnEvent(Event&) void
        +PushLayer(Layer*) void
        +PushOverlay(Layer*) void
        +static Get() Application&
        +GetWindow() Window&
        +GetFramebuffer() Framebuffer&
    }

    class TimeStep {
        -float m_Time
        +operator float()
        +GetSeconds() float
        +GetMilliseconds() float
    }

    class Renderer {
        -static SceneData* s_SceneData
        +static BeginScene(OrthographicCamera&) void
        +static Submit(VertexArray, Shader, mat4) void
        +static EndScene() void
        +static BlitToScreen(ComputeTexture) void
        +static GetAPI() API
    }

    class RenderCommand {
        -static RendererAPI* s_RendererAPI
        +static SetClearColor(vec4) void
        +static Clear() void
        +static SetViewport(x, y, w, h) void
        +static DrawIndexed(VertexArray) void
        +static DispatchCompute(gX, gY, gZ) void
        +static ComputeBarrier() void
        +static BlitToScreen(native, w, h) void
    }

    class RendererAPI {
        <<interface>>
        +SetClearColor(vec4)* void
        +Clear()* void
        +SetViewport(x, y, w, h)* void
        +DrawIndexed(VertexArray)* void
        +DispatchCompute(gX, gY, gZ)* void
        +ComputeBarrier()* void
        +BlitToScreen(native, w, h)* void
        +static GetAPI() API
    }

    class OrthographicCamera {
        -mat4 m_ProjectionMatrix
        -mat4 m_ViewMatrix
        -mat4 m_ViewProjectionMatrix
        -vec3 m_Position
        -float m_Rotation
        +SetPosition(vec3) void
        +SetRotation(float) void
        +GetViewProjectionMatrix() mat4
    }

    class PerspectiveCamera {
        -float m_Yaw
        -float m_Pitch
        -float m_MouseSensitivity
        -float m_Fov
        -vec3 m_CameraPosition
        -vec3 m_CameraDirection
        -vec3 m_CameraUp
        -vec3 m_CameraRight
        -float m_MovementSpeed
        +ProcessKeyboard(CameraMovement, float) void
        +ProcessMouseMovement(float, float, bool) void
        +ProcessMouseScroll(float) void
        +SetPosition(vec3) void
        +SetDirection(vec3) void
        +GetPosition() vec3
        +GetDirection() vec3
        +GetFov() float
    }

    class Shader {
        <<interface>>
        +Bind()* void
        +Unbind()* void
        +SetUniform1f(name, float)* void
        +SetUniform2f(name, vec2)* void
        +SetUniform3f(name, vec3)* void
        +SetUniform4f(name, vec4)* void
        +SetUniformMat3f(name, mat3)* void
        +SetUniformMat4f(name, mat4)* void
        +static Create(vertex, fragment, compute) Shader*
    }

    class ComputeShader {
        <<interface>>
        +Bind()* void
        +Unbind()* void
        +static Create(computePath) ComputeShader*
        #ReadFile(path) string
    }

    class ComputeTexture {
        <<interface>>
        +BindAsImage(unit)* void
        +BindAsTexture(slot)* void
        +Resize(w, h)* void
        +GetWidth()* uint32
        +GetHeight()* uint32
        +GetNativeHandle()* void*
        +static Create(w, h) ComputeTexture*
    }

    class Framebuffer {
        <<interface>>
        +Bind()* void
        +Unbind()* void
        +Resize(w, h)* void
        +GetColorAttachmentRendererID()* void*
        +GetSpecification()* FramebufferSpecification
        +static Create(spec) Framebuffer*
    }

    class VertexArray {
        <<interface>>
        +Bind()* void
        +Unbind()* void
        +AddVertexBuffer(VertexBuffer*)* void
        +SetIndexBuffer(IndexBuffer*)* void
        +static Create() VertexArray*
    }

    class StorageBuffer {
        <<interface>>
        +Bind(slot)* void
        +Unbind()* void
        +SetData(data, size, offset)* void
        +static Create(size, binding) StorageBuffer*
    }

    class Window {
        <<interface>>
        +OnUpdate()* void
        +GetWidth()* uint
        +GetHeight()* uint
        +GetFramebufferWidth()* uint
        +GetFramebufferHeight()* uint
        +GetTime()* float
        +SetEventCallback(fn)* void
        +SetVSync(bool)* void
        +IsVSync()* bool
        +HideCursor()* void
        +ShowCursor()* void
        +IsCursorHidden()* bool
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
        +OnUpdate(TimeStep) void
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
        +BeginDockspace() void
        +EndDockspace() void
        +RenderViewport() void
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

    class OpenGLContext
    class OpenGLRendererAPI
    class OpenGLShader
    class OpenGLVertexArray
    class OpenGLFramebuffer
    class OpenGLComputeShader
    class OpenGLComputeTexture

    class MetalContext
    class MetalRendererAPI
    class MetalShader
    class MetalVertexArray
    class MetalFramebuffer
    class MetalComputeShader
    class MetalComputeTexture
    class MetalStorageBuffer

    class WindowsInput
    class MacInput

    Application *-- LayerStack
    Application *-- Window
    Application *-- Framebuffer
    Application *-- TimeStep
    Application --> ImGuiLayer
    Application --> Log

    LayerStack o-- Layer
    ImGuiLayer --|> Layer

    Window <|.. WindowsWindow
    Window <|.. MacWindow
    GraphicsContext <|.. OpenGLContext
    GraphicsContext <|.. MetalContext
    RendererAPI <|.. OpenGLRendererAPI
    RendererAPI <|.. MetalRendererAPI
    Shader <|.. OpenGLShader
    Shader <|.. MetalShader
    VertexArray <|.. OpenGLVertexArray
    VertexArray <|.. MetalVertexArray
    Framebuffer <|.. OpenGLFramebuffer
    Framebuffer <|.. MetalFramebuffer
    ComputeShader <|.. OpenGLComputeShader
    ComputeShader <|.. MetalComputeShader
    ComputeTexture <|.. OpenGLComputeTexture
    ComputeTexture <|.. MetalComputeTexture
    StorageBuffer <|.. MetalStorageBuffer
    StorageBuffer <|.. OpenGLStorageBuffer
    Input <|.. WindowsInput
    Input <|.. MacInput

    WindowsWindow --> OpenGLContext
    MacWindow --> MetalContext

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

### Diagrama de Clases - Módulo Maze

```mermaid
classDiagram
    class Cell {
        -unsigned int m_Row
        -unsigned int m_Column
        -Cell* m_North
        -Cell* m_South
        -Cell* m_East
        -Cell* m_West
        -map~Cell*, bool~ m_Links
        +SetNorth(Cell*) void
        +SetSouth(Cell*) void
        +SetEast(Cell*) void
        +SetWest(Cell*) void
        +GetNorth() Cell*
        +GetSouth() Cell*
        +GetEast() Cell*
        +GetWest() Cell*
        +Link(Cell*, bool) void
        +Unlink(Cell*, bool) void
        +GetLinks() vector~Cell*~
        +IsLinked(Cell*) bool
        +GetNeighbors() vector~Cell*~
    }

    class Cell3D {
        -uint32_t m_Level
        -Cell3D* m_Up
        -Cell3D* m_Down
        +SetUp(Cell3D*) void
        +SetDown(Cell3D*) void
        +GetUp() Cell3D*
        +GetDown() Cell3D*
        +GetLevel() uint32_t
        +GetNeighbors() vector~Cell*~
    }

    class Grid {
        -unsigned int m_Rows
        -unsigned int m_Columns
        -vector~vector~Cell*~~ m_Grid
        +PrepareGrid() void
        +ConfigureCells() void
        +RandomCell() Cell*
        +EachRow(callback) void
        +EachCell(callback) void
        +Size() unsigned int
        +operator()(row, col) Cell*
        +ToString() string
    }

    class Grid3D {
        -uint32_t m_Rows
        -uint32_t m_Columns
        -uint32_t m_Levels
        -vector~vector~vector~Cell3D*~~~ m_Grid
        +PrepareGrid() void
        +ConfigureCells() void
        +RandomCell() Cell3D*
        +EachLevel(callback) void
        +EachRow(callback) void
        +EachCell(callback) void
        +Size() uint32_t
        +operator()(row, col, level) Cell3D*
        +ToString() string
        +ToIntMatrix3D() vector
        +static IntMatrix3DToString(matrix3D) string
    }

    class BinaryTree {
        +static On(Grid) Grid
        +static On(Grid3D*) void
    }

    class Wilsons {
        +static On(Grid) Grid
    }

    class Sidewinder {
        +static On(Grid) Grid
    }

    Cell3D --|> Cell : hereda
    Grid *-- Cell : contiene
    Grid3D *-- Cell3D : contiene
    BinaryTree ..> Grid : genera 2D
    BinaryTree ..> Grid3D : genera 3D
    BinaryTree ..> Cell : enlaza
    Wilsons ..> Grid : genera
    Wilsons ..> Cell : random walk
    Sidewinder ..> Grid : genera
    Sidewinder ..> Cell : enlaza por filas
```

### Diagrama de Clases - Módulo SVO

```mermaid
classDiagram
    class VoxelGrid {
        -vector~uint32_t~ m_Data
        -uint32_t m_Size
        +VoxelGrid(uint32_t size)
        +SetVoxel(x, y, z, material) void
        +GetVoxel(x, y, z) uint32_t
        +GetSize() uint32_t
        +ToString() string
    }

    class SVOBuilder {
        -vector~SVONode~ m_Nodes
        +Build(VoxelGrid) vector~SVONode~
        -BuildRecursive(nodeIndex, x, y, z, size, grid) OctantResult
    }

    class SVONode {
        +uint32_t descriptor
        +uint32_t material
    }

    class OctantResult {
        +bool isAir
        +bool isSolidLeaf
        +uint32_t leafMaterial
    }

    note for SVONode "descriptor: bits 0-7 Valid, 8-15 Leaf, 16-31 Child Index\nmaterial: RGBA color data\n8 bytes total = GPU-optimal packing"

    VoxelGrid --> SVOBuilder : input
    SVOBuilder --> SVONode : genera vector
    SVOBuilder --> OctantResult : resultado intermedio
```

### Flujo del Game Loop

```mermaid
sequenceDiagram
    participant EP as EntryPoint (main)
    participant App as Application
    participant Win as Window (GLFW)
    participant FB as Framebuffer
    participant LS as LayerStack
    participant GL as GameLayer
    participant Cam as PerspectiveCamera
    participant CS as ComputeShader
    participant CT as ComputeTexture
    participant SB as StorageBuffer (SVO)
    participant RC as RenderCommand
    participant R as Renderer
    participant IG as ImGuiLayer
    participant GC as GraphicsContext

    EP->>App: CreateApplication()
    EP->>App: Run()

    Note over GL: OnAttach(): Build VoxelGrid → SVOBuilder → StorageBuffer

    loop Game Loop (while m_Running)
        Note over App: Calcular TimeStep (delta time)
        App->>FB: Bind()

        loop Para cada Layer
            App->>LS: layer->OnUpdate(timeStep)
            GL->>Cam: ProcessKeyboard(WASD, dt)
            GL->>Cam: ProcessMouseMovement(dx, dy)
            GL->>CS: Bind()
            GL->>CT: BindAsImage(0)
            GL->>SB: Bind(1)
            Note over GL: Set uniforms: camera pos/dir/up/right, FOV, resolution, SVO size
            GL->>RC: DispatchCompute(groupsX, groupsY, 1)
            GL->>RC: ComputeBarrier()
            GL->>RC: SetClearColor(), Clear()
            GL->>R: BlitToScreen(computeTexture)
        end

        App->>FB: Unbind()

        App->>IG: Begin() + BeginDockspace()
        loop Para cada Layer
            App->>LS: layer->OnImGuiRender()
            Note over GL: Paneles: Stats, Maze 2D/3D, Camera, VoxelGrid
        end
        App->>IG: RenderViewport()
        App->>IG: EndDockspace() + End()

        App->>Win: OnUpdate()
        Win->>Win: glfwPollEvents()
        Win->>GC: SwapBuffers()
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

```text
Lore/                          # Raíz del workspace
├── premake5.lua               # Build system principal (Premake5)
├── Makefile                   # Makefile generado (macOS)
├── WindowsSetupPremake.bat    # Descarga Premake para Windows
├── WindowsSetupProject.bat    # Genera proyecto Visual Studio
│
├── Lore/                      # Motor (StaticLib)
│   ├── src/
│   │   ├── Lore.h             # Header público único (include todo)
│   │   ├── lrpch.h / .cpp     # Precompiled header
│   │   └── Lore/
│   │       ├── Application.h/.cpp    # Singleton, game loop, framebuffer, timestep
│   │       ├── Core.h               # Macros: plataforma, asserts, BIT()
│   │       ├── EntryPoint.h          # Define main()
│   │       ├── Input.h              # Singleton abstracto de input
│   │       ├── KeyCodes.h           # 120+ macros LR_KEY_*
│   │       ├── MouseButtonCodes.h   # 8 botones de ratón
│   │       ├── Layer.h/.cpp         # Clase base Layer (OnUpdate recibe TimeStep)
│   │       ├── LayerStack.h/.cpp    # Contenedor ordenado de capas
│   │       ├── Log.h/.cpp           # Wrapper spdlog (Core + Client)
│   │       ├── Window.h             # Interfaz pura de ventana (+Framebuffer sizes)
│   │       │
│   │       ├── Core/
│   │       │   └── TimeStep.h       # Delta time (seconds / milliseconds)
│   │       │
│   │       ├── Events/              # Sistema de eventos
│   │       │   ├── Event.h          # Base abstracta + EventDispatcher
│   │       │   ├── ApplicationEvent.h  # WindowClose, WindowResize, AppTick...
│   │       │   ├── KeyEvent.h       # KeyPressed, KeyReleased, KeyTyped
│   │       │   └── MouseEvent.h     # MouseMoved, MouseScrolled, MouseButton*
│   │       │
│   │       ├── ImGui/               # Integración Dear ImGui
│   │       │   ├── ImGuiLayer.h/.cpp      # Dockspace, viewport panel, auto-layout
│   │       │   ├── ImGuiBuild.cpp         # Unity build de backends ImGui
│   │       │   ├── ImGuiKeyCodes.h        # Mapeo LR_KEY → ImGuiKey
│   │       │   ├── ImGuiMetalBridge.h     # C++ bridge para imgui_impl_metal
│   │       │   └── ImGuiMetalBridge.mm    # Implementación Objective-C++
│   │       │
│   │       ├── Platform/            # Implementaciones por plataforma
│   │       │   ├── Windows/
│   │       │   │   ├── WindowsWindow.h/.cpp  # GLFW window para Windows
│   │       │   │   └── WindowsInput.h/.cpp   # Input polling para Windows
│   │       │   ├── Mac/
│   │       │   │   ├── MacWindow.h/.cpp      # GLFW window para macOS
│   │       │   │   └── MacInput.h/.cpp       # Input polling para macOS
│   │       │   ├── OpenGL/
│   │       │   │   ├── OpenGLContext.h/.cpp          # Init GLAD + swap buffers
│   │       │   │   ├── OpenGLRendererAPI.h/.cpp      # OpenGL draw + compute dispatch
│   │       │   │   ├── OpenGLShader.h/.cpp           # GLSL compilación + uniforms
│   │       │   │   ├── OpenGLBuffer.h/.cpp           # VBO / IBO OpenGL
│   │       │   │   ├── OpenGLVertexArray.h/.cpp      # VAO OpenGL
│   │       │   │   ├── OpenGLFramebuffer.h/.cpp      # FBO con color + depth/stencil
│   │       │   │   ├── OpenGLComputeShader.h/.cpp    # GL_COMPUTE_SHADER dispatch
│   │       │   │   └── OpenGLComputeTexture.h/.cpp   # glTexStorage2D + image binding
│   │       │   └── Metal/
│   │       │       ├── MetalContext.h/.mm             # MTLDevice, CommandQueue, CAMetalLayer
│   │       │       ├── MetalRendererAPI.h/.mm         # Draw + compute + blit pipeline
│   │       │       ├── MetalShader.h/.mm              # MSL compilación + render pipeline
│   │       │       ├── MetalBuffer.h/.mm              # MTLBuffer (vertex/index/storage)
│   │       │       ├── MetalVertexArray.h/.mm         # Tracking de buffers (sin VAO nativo)
│   │       │       ├── MetalFramebuffer.h/.mm         # Offscreen MTLTexture
│   │       │       ├── MetalComputeShader.h/.mm       # MTLComputePipelineState
│   │       │       └── MetalComputeTexture.h/.mm      # MTLTexture para compute read/write
│   │       │
│   │       └── Renderer/            # Sistema de renderizado (interfaces)
│   │       │   ├── GraphicsContext.h    # Interfaz pura (Init + SwapBuffers)
│   │       │   ├── Renderer.h/.cpp      # BeginScene, Submit, EndScene, BlitToScreen
│   │       │   ├── RenderCommand.h      # Comandos estáticos: Draw, Compute, Blit
│   │       │   ├── RendererAPI.h/.cpp   # Abstracción de API gráfica (None/OpenGL/Metal)
│   │       │   ├── RenderAPI.cpp        # Selección de API por plataforma
│   │       │   ├── Shader.h/.cpp        # Interfaz de shaders + file loading
│   │       │   ├── Buffer.h/.cpp        # VertexBuffer, IndexBuffer, StorageBuffer, BufferLayout
│   │       │   ├── VertexArray.h/.cpp   # Vertex Array Objects
│   │       │   ├── Framebuffer.h/.cpp   # Offscreen render target
│   │       │   ├── ComputeShader.h/.cpp # GPGPU compute dispatch
│   │       │   ├── ComputeTexture.h/.cpp# Texture para compute read/write
│   │       │   ├── OrthographicCamera.h/.cpp  # Cámara 2D con transformaciones
│   │       │   └── PerspectiveCamera.h/.cpp   # Cámara FPS (Yaw/Pitch/WASD/Mouse)
│   │
│   └── vendor/                # Dependencias del motor
│       ├── GLFW/              # Windowing y input nativo
│       ├── GLAD/              # OpenGL function loader
│       ├── IMGUI/             # Dear ImGui (rama docking)
│       ├── glm/               # Matemáticas (vectores, matrices, transformaciones)
│       └── spdlog/            # Logging rápido (header-only)
│
├── Sandbox/                   # Aplicación de ejemplo (ConsoleApp)
│   └── src/
│       ├── SandboxApp.cpp     # Entry point: crea GameLayer
│       ├── sndbxpch.h / .cpp  # Precompiled header del Sandbox
│       ├── Game/              # Capa principal de juego
│       │   ├── GameLayer.h          # Declaración: cámara, SVO, maze, compute
│       │   ├── GameLayer.cpp        # OnAttach, OnUpdate, OnEvent, controles FPS
│       │   ├── GamLayer_ImGUI.cpp   # OnImGuiRender: paneles Stats/Maze/Camera/Voxel
│       │   └── GameObject.h         # Placeholder para sistema de objetos
│       ├── Shaders/           # Shaders duales por plataforma
│       │   ├── OpenGL/
│       │   │   ├── Main.vertex.glsl    # Vertex shader GLSL
│       │   │   ├── Main.fragment.glsl  # Fragment shader GLSL
│       │   │   └── Main.compute.glsl   # Compute shader: SVO raytracing
│       │   └── Metal/
│       │       ├── Main.vertex.metal   # Vertex shader MSL
│       │       ├── Main.fragment.metal # Fragment shader MSL
│       │       └── Main.compute.metal  # Compute shader: SVO raytracing
│       ├── Maze/              # Sistema de generación de laberintos
│       │   ├── Base/
│       │   │   ├── Cell.h/.cpp     # Celda 2D con enlaces N/S/E/W
│       │   │   ├── Cell3D.h/.cpp   # Celda 3D: hereda Cell + Up/Down
│       │   │   ├── Grid.h/.cpp     # Matriz 2D + iteradores + ToString()
│       │   │   └── Grid3D.h/.cpp   # Matriz 3D + ToIntMatrix3D()
│       │   └── Algorithms/
│       │       ├── BinaryTree.h/.cpp   # Generador con sesgo NE (2D y 3D)
│       │       ├── Wilsons.h/.cpp      # Random walk sin loops
│       │       └── Sidewinder.h/.cpp   # Generación por filas
│       └── SVO/               # Sparse Voxel Octree
│           ├── VoxelGrid.h    # Grid volumétrico 256³ en CPU
│           └── SVO.h          # SVOBuilder + SVONode (8 bytes GPU-packed)
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

```text
GraphicsContext (interfaz)
├── OpenGLContext
└── MetalContext

RendererAPI (interfaz)
├── OpenGLRendererAPI
└── MetalRendererAPI

Shader (interfaz + factory)
├── OpenGLShader
└── MetalShader

ComputeShader (interfaz + factory)
├── OpenGLComputeShader
└── MetalComputeShader

ComputeTexture (interfaz + factory)
├── OpenGLComputeTexture
└── MetalComputeTexture

Framebuffer (interfaz + factory)
├── OpenGLFramebuffer
└── MetalFramebuffer

VertexBuffer (interfaz + factory)
├── OpenGLVertexBuffer
└── MetalVertexBuffer

IndexBuffer (interfaz + factory)
├── OpenGLIndexBuffer
└── MetalIndexBuffer

StorageBuffer (interfaz + factory)
├── OpenGLStorageBuffer
└── MetalStorageBuffer

VertexArray (interfaz + factory)
├── OpenGLVertexArray
└── MetalVertexArray

OrthographicCamera
└── (Clase concreta con glm)

PerspectiveCamera
└── (Clase concreta FPS: Yaw/Pitch, WASD, Mouse, FOV)

TimeStep
└── (Clase concreta, delta time)

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
└── ImGuiLayer (dockspace + viewport panel)

Application (singleton, el cliente hereda de esta)
└── Sandbox (código cliente)
    └── GameLayer (voxel raytracing FPS demo)

Maze (módulo independiente en Sandbox)
├── Grid (matriz 2D de celdas)
│   └── Cell (celda con enlaces N/S/E/W)
├── Grid3D (matriz 3D de celdas)
│   └── Cell3D (celda con N/S/E/W + Up/Down, hereda de Cell)
└── Algorithms
    ├── BinaryTree (sesgo hacia esquina NE, soporta 2D y 3D)
    ├── Wilsons (random walk sin loops)
    └── Sidewinder (generación por filas)

SVO (módulo de Sparse Voxel Octree en Sandbox)
├── VoxelGrid (grid volumétrico 256³, CPU-side)
├── SVOBuilder (construcción recursiva del octree)
└── SVONode (nodo empaquetado de 8 bytes para GPU)
```

---

## Dependencias

| Librería                                       | Versión / Rama | Propósito                                                       |
| ---------------------------------------------- | -------------- | --------------------------------------------------------------- |
| [GLFW](https://www.glfw.org/)                  | —              | Creación de ventanas, contexto gráfico, input nativo            |
| [GLAD](https://glad.dav1d.de/)                 | OpenGL 4.1     | Loader de funciones OpenGL (solo Windows)                       |
| [Dear ImGui](https://github.com/ocornut/imgui) | Rama docking   | GUI inmediata con docking y multi-viewports                     |
| [spdlog](https://github.com/gabime/spdlog)     | Header-only    | Logging rápido con formato y colores                            |
| [glm](https://github.com/g-truc/glm)           | —              | Matemáticas (vectores, matrices) para cámara y transformaciones |
| [Premake5](https://premake.github.io/)         | —              | Generación de proyectos (VS, Makefiles)                         |
| Apple Metal.framework                          | —              | API de renderizado nativa en macOS                              |
| Apple MetalKit.framework                       | —              | Utilidades Metal (capas, vistas)                                |
| Apple QuartzCore.framework                     | —              | CAMetalLayer para presentación en pantalla                      |

---

## Estado del Proyecto

### Implementado

| Módulo                       | Estado    | Descripción                                                                                      |
| ---------------------------- | --------- | ------------------------------------------------------------------------------------------------ |
| Windowing (GLFW)             | Completo  | Creación de ventana, VSync, callbacks. Windows + Mac. Soporte Retina/HiDPI                       |
| Sistema de Eventos           | Completo  | Despacho inmediato por tipo con `EventDispatcher`. 14 tipos de evento                            |
| Input Polling                | Completo  | Teclado + ratón, ambas plataformas                                                               |
| Logging (spdlog)             | Completo  | Logger dual Core (`LORE`) / Client (`APP`) con macros                                            |
| Sistema de Capas             | Completo  | `LayerStack` con capas normales y overlays. Propagación inversa de eventos                       |
| Integración ImGui            | Completo  | Dockspace con auto-layout, viewport de framebuffer, backends GLFW + OpenGL3 + Metal              |
| Contexto OpenGL              | Completo  | OpenGL 4.1, GLAD loader, info de GPU al inicio                                                   |
| **Backend Metal**            | Completo  | Apple Metal completo: device, command queue, render/compute pipelines, blit a pantalla           |
| **Selección de API**         | Completo  | Automática por plataforma: OpenGL en Windows, Metal en macOS                                     |
| **Renderer Abstraction**     | Completo  | Renderer, RenderCommand, RendererAPI con Draw + Compute + Blit                                   |
| **Shader Abstraction**       | Completo  | Interfaz unificada con uniforms (float, vec2-4, mat3-4), factory por plataforma                  |
| **Buffer Abstraction**       | Completo  | VertexBuffer, IndexBuffer con `GetNativeHandle()`, StorageBuffer (OpenGL SSBO + Metal MTLBuffer) |
| **Framebuffer**              | Completo  | Offscreen rendering con resize dinámico, integrado en Application y viewport ImGui               |
| **Compute Shader Pipeline**  | Completo  | Dispatch de compute shaders con texturas writable/readable y blit full-screen                    |
| **Compute Texture**          | Completo  | RGBA8 textures con `BindAsImage()` / `BindAsTexture()`, resize dinámico                          |
| **Cámara Ortográfica**       | Completo  | OrthographicCamera con posición, rotación, matrices View/Projection                              |
| **Cámara Perspectiva (FPS)** | Completo  | PerspectiveCamera con Yaw/Pitch, WASD, mouse look, FOV scroll, velocidad configurable            |
| **TimeStep (Delta Time)**    | Completo  | Clase `TimeStep` pasada a `Layer::OnUpdate()` cada frame                                         |
| **Control de Cursor**        | Completo  | `HideCursor()`/`ShowCursor()`/`IsCursorHidden()` para modo FPS con captura de ratón              |
| **glm Integration**          | Completo  | Matrices y vectores para transformaciones de cámara y uniforms                                   |
| **ImGui Metal Bridge**       | Completo  | C++ bridge para imgui_impl_metal con Init, Shutdown, NewFrame, RenderDrawData                    |
| Compute-First Demo           | Funcional | Pipeline compute → SVO raytracing → blit → ImGui con paneles interactivos                        |

### En Progreso / Pendiente

| Módulo                        | Estado              | Notas                                                                 |
| ----------------------------- | ------------------- | --------------------------------------------------------------------- |
| Event Buffering               | **No implementado** | Los eventos se despachan inmediatamente (trabajo futuro en `Event.h`) |
| Eventos WindowFocus/Move      | **No implementado** | Los enum values existen pero no hay callbacks GLFW que los disparen   |
| Texturas (2D)                 | **No iniciado**     | No hay sistema de carga/binding de texturas convencionales            |
| Renderer2D                    | **No iniciado**     | Batching de sprites/quads                                             |
| Scene Graph                   | **No iniciado**     | —                                                                     |
| ECS (Entity Component System) | **No iniciado**     | —                                                                     |
| GameObject                    | **Placeholder**     | Header vacío, futuro sistema de objetos de juego                      |
| Audio                         | **No iniciado**     | —                                                                     |
| Física                        | **No iniciado**     | —                                                                     |

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

    section Renderer Core
    Shader Abstraction        :done, 2026-01, 2026-02
    Buffer Objects (VBO/VAO)  :done, 2026-01, 2026-02
    Render Commands           :done, 2026-02, 2026-02
    Cámara Ortográfica        :done, 2026-02, 2026-02
    TimeStep / Delta Time     :done, 2026-02, 2026-03

    section Multi-API
    Metal Backend             :done, 2026-02, 2026-03
    API Auto-Selection        :done, 2026-02, 2026-03
    ImGui Metal Bridge        :done, 2026-02, 2026-03

    section Compute Pipeline
    ComputeShader             :done, 2026-02, 2026-03
    ComputeTexture            :done, 2026-02, 2026-03
    Framebuffer               :done, 2026-02, 2026-03
    Full-Screen Blit          :done, 2026-02, 2026-03
    ImGui Dockspace+Viewport  :done, 2026-02, 2026-03

    section Cámara y Tiempo
    StorageBuffer (SSBO)      :done, 2026-03, 2026-03
    PerspectiveCamera FPS     :done, 2026-03, 2026-03
    Control de Cursor          :done, 2026-03, 2026-03

    section Voxel System
    VoxelGrid (256³)          :done, 2026-03, 2026-03
    SVOBuilder                :done, 2026-03, 2026-03
    SVO Raytracing (Compute)  :done, 2026-03, 2026-03
    Maze 3D (Grid3D/Cell3D)   :done, 2026-03, 2026-03

    section Próximo
    Texturas 2D               :active, 2026-04, 2026-06
    Cámara 2D/3D unificada    : 2026-05, 2026-07
    Renderer2D                : 2026-07, 2026-09

    section Motor de Juego
    Scene Graph               : 2026-08, 2026-10
    ECS                       : 2026-09, 2026-12
    Sistema Voxel (SVO)       : 2027-01, 2027-06
```

---

## Sandbox - Aplicación de Ejemplo

El proyecto **Sandbox** es una aplicación de demostración que muestra las capacidades del motor Lore Engine con un **pipeline compute-first de voxel raytracing**.

### GameLayer

La capa principal (`Game/GameLayer`) que implementa:

- **Voxel Raytracing por SVO**: Pipeline completo de compute shader que recorre un Sparse Voxel Octree en GPU
- **Cámara FPS en Perspectiva**: Movimiento WASD, mouse look (Yaw/Pitch), scroll para FOV, velocidad ajustable desde ImGui
- **Sparse Voxel Octree**: VoxelGrid de 256³ → SVOBuilder recursivo → StorageBuffer GPU con nodos de 8 bytes
- **Framebuffer Offscreen**: Renderizado a textura, visualizado en viewport ImGui con dockspace
- **Generación de Laberintos 2D/3D**: Tres algoritmos (BinaryTree, Wilson's, Sidewinder) con UI interactiva; BinaryTree también soporta Grid3D
- **Resize Dinámico**: Compute texture y framebuffer se redimensionan con la ventana
- **FPS Counter**: Estadísticas de rendimiento en tiempo real via TimeStep
- **Control de Cursor**: Click para capturar ratón (modo FPS), ESC para liberar

### Controles

| Elemento                | Acción                                                                                |
| ----------------------- | ------------------------------------------------------------------------------------- |
| **WASD**                | Movimiento de cámara (Forward/Left/Backward/Right)                                    |
| **Ratón**               | Rotación de cámara (Yaw/Pitch) cuando el cursor está capturado                        |
| **Scroll**              | Ajustar FOV (zoom)                                                                    |
| **Click izquierdo**     | Capturar cursor (entrar en modo FPS)                                                  |
| **ESC**                 | Liberar cursor (salir de modo FPS)                                                    |
| Panel "Stats"           | Resolución, framebuffer size y FPS                                                    |
| Panel "Maze"            | Generación de laberintos 2D (Binary Tree, Wilson's, Sidewinder) y 3D (Binary Tree 3D) |
| Slider "Movement Speed" | Velocidad de movimiento de cámara                                                     |
| Slider "Camera angle"   | FOV de la cámara                                                                      |
| Panel "Voxel Grid"      | Info del VoxelGrid: tamaño total, bloques no-aire, memoria                            |
| Panel "Viewport"        | Muestra el framebuffer con el resultado del compute shader                            |

### Módulo SVO

Sistema de Sparse Voxel Octree para representación eficiente de datos volumétricos en GPU.

#### Estructura

```text
SVO/
├── VoxelGrid.h   # Grid volumétrico temporal (CPU)
└── SVO.h         # SVOBuilder + SVONode
```

#### Clases

**VoxelGrid** - Grid volumétrico en CPU:

- Tamaño configurable (potencia de 2, por defecto 256³)
- Almacena `uint32_t` material por voxel (0 = aire)
- `SetVoxel(x, y, z, material)` / `GetVoxel(x, y, z)`
- `ToString()` con estadísticas de memoria y bloques

**SVOBuilder** - Construye el octree recursivamente:

- `Build(VoxelGrid)` → `vector<SVONode>` listo para GPU
- Recorre el grid dividiendo en octantes recursivamente
- Detecta nodos vacíos (aire) y hojas sólidas para comprimir

**SVONode** - Nodo empaquetado de 8 bytes (64 bits):

- `descriptor`: bits 0-7 Valid mask, bits 8-15 Leaf mask, bits 16-31 Child Index
- `material`: color RGBA o propiedad del material
- Tamaño óptimo para acceso en GPU (alineado a 8 bytes)

### Módulo Maze

Sistema de generación procedural de laberintos 2D y 3D implementado como ejemplo de lógica de juego independiente del motor.

#### Estructura

```text
Maze/
├── Base/
│   ├── Cell.h/.cpp       # Celda 2D individual del laberinto
│   ├── Cell3D.h/.cpp     # Celda 3D (hereda Cell + Up/Down)
│   ├── Grid.h/.cpp       # Matriz 2D de celdas
│   └── Grid3D.h/.cpp     # Matriz 3D de celdas
└── Algorithms/
    ├── BinaryTree.h/.cpp   # Sesgo hacia esquina NE (2D y 3D)
    ├── Wilsons.h/.cpp      # Random walk sin loops
    └── Sidewinder.h/.cpp   # Generación por filas
```

#### Clases

**Cell** - Representa una celda 2D del laberinto:

- Coordenadas (fila, columna)
- Referencias a vecinos (North, South, East, West)
- Sistema de enlaces bidireccionales entre celdas
- Métodos: `Link()`, `Unlink()`, `IsLinked()`, `GetNeighbors()`

**Cell3D** - Celda 3D que hereda de Cell:

- Añade dimensión vertical: `Up` y `Down` además de N/S/E/W
- Coordenada de nivel (`m_Level`)
- Override de `GetNeighbors()` para incluir vecinos verticales

**Grid** - Matriz 2D contenedora de celdas:

- Dimensiones configurables (filas × columnas)
- Iteradores `EachRow()` y `EachCell()` con callbacks
- Acceso por coordenadas con `operator()(row, col)`
- Método `ToString()` para representación ASCII
- Método `RandomCell()` para selección aleatoria

**Grid3D** - Matriz 3D contenedora de celdas:

- Dimensiones (filas × columnas × niveles)
- Iteradores `EachLevel()`, `EachRow()` y `EachCell()` con callbacks
- Acceso por coordenadas con `operator()(row, col, level)`
- `ToIntMatrix3D()` para representación como matriz de enteros
- `IntMatrix3DToString()` para visualización de la matriz 3D
- Método `ToString()` para representación ASCII por niveles

#### Algoritmos

**BinaryTree** - Generador con sesgo direccional:

- Método estático `On(Grid)` para grids 2D y `On(Grid3D*)` para grids 3D
- Para cada celda, elige aleatoriamente entre Norte o Este (y Up en 3D)
- Crea enlaces (pasajes) entre celdas adyacentes
- Produce laberintos con sesgo hacia esquina NE

**Wilsons** - Random walk sin loops (loop-erased):

- Método estático `On(Grid)` que procesa el grid
- Comienza con todas las celdas sin visitar
- Realiza caminatas aleatorias desde celdas no visitadas
- Borra loops cuando revisita una celda en el path actual
- Conecta cuando alcanza una celda ya visitada
- Produce laberintos uniformes (sin sesgo direccional)

**Sidewinder** - Generación fila a fila:

- Método estático `On(Grid)` que procesa el grid
- Mantiene una "run" de celdas por fila
- Cierra runs aleatoriamente enlazando un miembro hacia el Norte
- De lo contrario, enlaza hacia el Este
- Produce laberintos con sesgo horizontal

#### Ejemplo de Uso

```cpp
#include "Maze/Algorithms/BinaryTree.h"
#include "Maze/Algorithms/Wilsons.h"
#include "Maze/Algorithms/Sidewinder.h"

// === Laberinto 2D ===
Maze::Grid grid{ 50, 50 };
grid = Maze::BinaryTree::On(grid);   // sesgo NE
// grid = Maze::Wilsons::On(grid);   // uniforme, sin sesgo
// grid = Maze::Sidewinder::On(grid); // sesgo horizontal
std::string mazeString = grid.ToString();

// === Laberinto 3D ===
Maze::Grid3D grid3D(5, 5, 5);
Maze::BinaryTree::On(&grid3D);
std::string maze3DString = grid3D.ToString();

// Convertir a matriz de enteros (útil para voxelización)
auto intMatrix = grid3D.ToIntMatrix3D();
std::string matrixStr = Maze::Grid3D::IntMatrix3DToString(intMatrix);

// Mostrar en ImGui
ImGui::Text("%s", mazeString.c_str());
```

#### Salida de Ejemplo

```text
+---+---+---+---+---+
|               |   |
+   +---+---+   +   +
|   |       |   |   |
+   +   +---+   +   +
|   |   |   |       |
+---+---+---+---+---+
```

---

## Building

### Requisitos Previos

- Compilador C++23 o superior
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
#include <Lore/Renderer/PerspectiveCamera.h>
#include <imgui.h>

class GameLayer : public Lore::Layer {
private:
    std::shared_ptr<Lore::ComputeShader> m_ComputeShader;
    std::shared_ptr<Lore::ComputeTexture> m_ComputeTexture;
    std::shared_ptr<Lore::StorageBuffer> m_SVOBuffer;
    Lore::PerspectiveCamera m_Camera;
    uint32_t m_Width = 800, m_Height = 600;

public:
    GameLayer() : Layer("Game") {
#ifdef LORE_PLATFORM_MAC
        m_ComputeShader.reset(Lore::ComputeShader::Create("path/to/shader.metal"));
#elif defined(LORE_PLATFORM_WINDOWS)
        m_ComputeShader.reset(Lore::ComputeShader::Create("path/to/shader.glsl"));
#endif
    }

    void OnAttach() override {
        m_Width = Lore::Application::Get().GetWindow().GetFramebufferWidth();
        m_Height = Lore::Application::Get().GetWindow().GetFramebufferHeight();
        m_ComputeTexture.reset(Lore::ComputeTexture::Create(m_Width, m_Height));

        // Build SVO y subir a GPU via StorageBuffer
        std::vector<uint32_t> svoData = BuildSVO();
        m_SVOBuffer.reset(Lore::StorageBuffer::Create(svoData.size() * sizeof(uint32_t), 1));
        m_SVOBuffer->SetData(svoData.data(), svoData.size() * sizeof(uint32_t));
    }

    void OnUpdate(Lore::TimeStep ts) override {
        // Cámara FPS
        if (Lore::Input::IsKeyPressed(LR_KEY_W))
            m_Camera.ProcessKeyboard(Lore::CameraMovement::FORWARD, ts);
        if (Lore::Input::IsKeyPressed(LR_KEY_S))
            m_Camera.ProcessKeyboard(Lore::CameraMovement::BACKWARD, ts);

        // Pipeline compute-first con SVO
        m_ComputeShader->Bind();
        m_ComputeTexture->BindAsImage(0);
        m_SVOBuffer->Bind(1);

        // Set camera uniforms...
        uint32_t groupsX = (m_Width + 15) / 16;
        uint32_t groupsY = (m_Height + 15) / 16;
        Lore::RenderCommand::DispatchCompute(groupsX, groupsY, 1);
        Lore::RenderCommand::ComputeBarrier();

        Lore::RenderCommand::SetClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });
        Lore::RenderCommand::Clear();
        Lore::Renderer::BlitToScreen(m_ComputeTexture);
    }

    void OnImGuiRender() override {
        ImGui::Begin("Debug");
        ImGui::SliderFloat("Speed", m_Camera.GetMovemetSpeedPtr(), 0.0f, 20.0f);
        ImGui::SliderFloat("FOV", m_Camera.GetFovPtr(), 0.0f, 90.0f);
        ImGui::End();
    }

    void OnEvent(Lore::Event& event) override {
        Lore::EventDispatcher dispatcher(event);
        dispatcher.Dispatch<Lore::WindowResizeEvent>(
            [this](Lore::WindowResizeEvent& e) {
                m_Width = e.GetWidth();
                m_Height = e.GetHeight();
                m_ComputeTexture->Resize(m_Width, m_Height);
                return false;
            });
        dispatcher.Dispatch<Lore::KeyPressedEvent>(
            [this](Lore::KeyPressedEvent& e) {
                if (e.GetKeyCode() == LR_KEY_ESCAPE) {
                    Lore::Application::Get().GetWindow().ShowCursor();
                }
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

> **Nota**: `ImGuiLayer` se añade automáticamente como overlay en el constructor de `Application`. El framebuffer offscreen y el dockspace con viewport se gestionan automáticamente.

### Pipeline Tradicional (Raster)

El motor también soporta el pipeline tradicional con vertex/fragment shaders:

```cpp
void OnUpdate(Lore::TimeStep ts) override {
    // Control de cámara con delta time
    if (Lore::Input::IsKeyPressed(LR_KEY_LEFT))
        m_CameraPosition.x -= 2.0f * ts;

    m_Camera.SetPosition(m_CameraPosition);

    Lore::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
    Lore::RenderCommand::Clear();

    Lore::Renderer::BeginScene(m_Camera);
    Lore::Renderer::Submit(m_VertexArray, m_Shader, transform);
    Lore::Renderer::EndScene();
}
```

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

### Sistema de Renderizado

```cpp
// === Pipeline Compute-First ===

// Crear compute shader (plataforma automática)
auto computeShader = std::shared_ptr<Lore::ComputeShader>(
    Lore::ComputeShader::Create("path/to/shader.metal"));  // o .glsl

// Crear textura de compute (RGBA8)
auto computeTexture = std::shared_ptr<Lore::ComputeTexture>(
    Lore::ComputeTexture::Create(width, height));

// Crear StorageBuffer para datos arbitrarios (ej. SVO)
auto storageBuffer = std::shared_ptr<Lore::StorageBuffer>(
    Lore::StorageBuffer::Create(dataSize, bindingPoint));
storageBuffer->SetData(data.data(), dataSize);

// En el game loop (OnUpdate)
computeShader->Bind();
computeTexture->BindAsImage(0);  // Writable para compute
storageBuffer->Bind(1);          // Datos leíbles por compute
Lore::RenderCommand::DispatchCompute(groupsX, groupsY, 1);
Lore::RenderCommand::ComputeBarrier();

Lore::RenderCommand::SetClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });
Lore::RenderCommand::Clear();
Lore::Renderer::BlitToScreen(computeTexture);  // Full-screen triangle blit

// Resize dinámico
computeTexture->Resize(newWidth, newHeight);
```

```cpp
// === Pipeline Raster Tradicional ===

// Crear recursos
auto vertexArray = std::shared_ptr<Lore::VertexArray>(Lore::VertexArray::Create());
auto vertexBuffer = std::shared_ptr<Lore::VertexBuffer>(
    Lore::VertexBuffer::Create(vertices, sizeof(vertices)));

vertexBuffer->SetLayout({
    { Lore::ShaderDataType::Float3, "a_Position" },
    { Lore::ShaderDataType::Float4, "a_Color" },
});
vertexArray->AddVertexBuffer(vertexBuffer);

auto indexBuffer = std::shared_ptr<Lore::IndexBuffer>(
    Lore::IndexBuffer::Create(indices, count));
vertexArray->SetIndexBuffer(indexBuffer);

// Crear shader (vertex + fragment, carga desde archivo)
auto shader = std::shared_ptr<Lore::Shader>(
    Lore::Shader::Create(vertexPath, fragmentPath, computePath));

// Crear cámara
Lore::OrthographicCamera camera(-1.6f, 1.6f, -0.9f, 0.9f);
camera.SetPosition({ 0.0f, 0.0f, 0.0f });
camera.SetRotation(0.0f);

// En el game loop (OnUpdate)
Lore::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
Lore::RenderCommand::Clear();

Lore::Renderer::BeginScene(camera);
Lore::Renderer::Submit(vertexArray, shader, glm::mat4(1.0f));
Lore::Renderer::EndScene();
```

```cpp
// === Framebuffer Offscreen ===

// Acceder al framebuffer de la aplicación
Lore::Framebuffer& fb = Lore::Application::Get().GetFramebuffer();
fb.Bind();
// ... renderizar escena ...
fb.Unbind();  // Vuelve al render target de pantalla

// Obtener textura del framebuffer (para ImGui viewport, etc.)
void* colorAttachment = fb.GetColorAttachmentRendererID();
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

| Patrón              | Uso                                                                                                                                                                                                  |
| ------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Singleton**       | `Application`, `Input`, `MetalContext` — acceso global a instancia única                                                                                                                             |
| **Factory Method**  | `Window::Create()`, `VertexArray::Create()`, `Buffer::Create()`, `StorageBuffer::Create()`, `Framebuffer::Create()`, `ComputeShader::Create()`, `ComputeTexture::Create()` — creación por plataforma |
| **Observer**        | Sistema de eventos — callbacks GLFW → `Event` → `Application` → `Layer`                                                                                                                              |
| **Template Method** | `Layer` — define hooks virtuales que las subclases implementan                                                                                                                                       |
| **Strategy**        | `Input`, `Window`, `RendererAPI`, `GraphicsContext` — interfaz común con implementaciones intercambiables (OpenGL/Metal)                                                                             |
| **Composite**       | `LayerStack` — colección ordenada de `Layer` con iteración uniforme                                                                                                                                  |
| **Command**         | `RenderCommand` — encapsula operaciones de renderizado/compute/blit como métodos estáticos                                                                                                           |
| **Bridge**          | `ImGuiMetalBridge` — C++ bridge sobre Objective-C para imgui_impl_metal                                                                                                                              |

---

## Licencia

Este proyecto está bajo la Licencia MIT — ver el archivo [LICENSE](LICENSE) para más detalles.

Copyright (c) 2026 Bruno Vitte
