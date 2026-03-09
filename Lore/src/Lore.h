#pragma once

// * Will be use by Lore applications *

#include "Lore/Application.h"
#include "Lore/Layer.h"
#include "Lore/Log.h"

#include "Lore/Core/TimeStep.h"

#include "Lore/MouseButtonCodes.h"
#include "Lore/KeyCodes.h"
#include "Lore/Input.h"


#include "Lore/ImGui/ImGuiLayer.h"

// ! Renderer !
#include "Lore/Renderer/Renderer.h"
#include "Lore/Renderer/RenderCommand.h"

#include "Lore/Renderer/Buffer.h"
#include "Lore/Renderer/Shader.h"
#include "Lore/Renderer/VertexArray.h"
#include "Lore/Renderer/Framebuffer.h"

#include "Lore/Renderer/ComputeTexture.h"
#include "Lore/Renderer/ComputeShader.h"

#include "Lore/Renderer/OrthographicCamera.h"
#include "Lore/Renderer/PerspectiveCamera.h"
// ! ******** !

// ! Entry Point !
// ? Must be include direct on the first line of the application's main file (Sandbox/src/SandboxApp.cpp) to avoid linker errors due to multiple definitions of the entry point function.
// ! *********** !
