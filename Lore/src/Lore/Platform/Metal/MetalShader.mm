#include "lrpch.h"

#ifdef LORE_PLATFORM_MAC

#include "MetalShader.h"
#include "MetalContext.h"
#include "MetalBuffer.h"

#import <Metal/Metal.h>

namespace Lore {

	// Helper: convert ShaderDataType to MTLVertexFormat
	static MTLVertexFormat ShaderDataTypeToMetalFormat(ShaderDataType type) {
		switch (type) {
			case ShaderDataType::Float:   return MTLVertexFormatFloat;
			case ShaderDataType::Float2:  return MTLVertexFormatFloat2;
			case ShaderDataType::Float3:  return MTLVertexFormatFloat3;
			case ShaderDataType::Float4:  return MTLVertexFormatFloat4;
			case ShaderDataType::Int:     return MTLVertexFormatInt;
			case ShaderDataType::Int2:    return MTLVertexFormatInt2;
			case ShaderDataType::Int3:    return MTLVertexFormatInt3;
			case ShaderDataType::Int4:    return MTLVertexFormatInt4;
			case ShaderDataType::Bool:    return MTLVertexFormatChar;
			default: break;
		}
		LR_CORE_ASSERT(false, "Unknown ShaderDataType for Metal!");
		return MTLVertexFormatInvalid;
	}

	MetalShader::MetalShader(const std::string& vertexPath, const std::string& fragmentPath, const std::string& computeShaderPath) {
		MetalContext* context = MetalContext::Get();
		LR_CORE_ASSERT(context, "No active Metal context!");

		id<MTLDevice> device = (__bridge id<MTLDevice>)context->GetDevice();

		// Read and combine shader sources into a single Metal library source
		std::string shaderSource = "";

		if (!vertexPath.empty()) {
			shaderSource += ReadFile(vertexPath);
			shaderSource += "\n";
		}
		if (!fragmentPath.empty()) {
			shaderSource += ReadFile(fragmentPath);
			shaderSource += "\n";
		}

		// Compile the shader source
		NSString* source = [NSString stringWithUTF8String:shaderSource.c_str()];
		NSError* error = nil;

		MTLCompileOptions* options = [[MTLCompileOptions alloc] init];
		id<MTLLibrary> library = [device newLibraryWithSource:source options:options error:&error];

		if (error) {
			LR_CORE_ERROR("Metal shader compilation failed: {0}",
				std::string([[error localizedDescription] UTF8String]));
			LR_CORE_ASSERT(false, "Metal shader compilation failed!");
			return;
		}
		m_Library = (__bridge_retained void*)library;

		// Get the vertex and fragment functions
		id<MTLFunction> vertexFunction = [library newFunctionWithName:@"vertex_main"];
		id<MTLFunction> fragmentFunction = [library newFunctionWithName:@"fragment_main"];

		LR_CORE_ASSERT(vertexFunction, "Failed to find vertex_main function in Metal shader!");
		LR_CORE_ASSERT(fragmentFunction, "Failed to find fragment_main function in Metal shader!");

		// Create render pipeline descriptor
		MTLRenderPipelineDescriptor* pipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
		pipelineDescriptor.vertexFunction = vertexFunction;
		pipelineDescriptor.fragmentFunction = fragmentFunction;
		pipelineDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;

		// Build vertex descriptor dynamically from BufferLayout
		// NOTE: The vertex descriptor will be rebuilt if needed when a layout is provided.
		// For now, we create a default that matches a common layout (Float3 pos + Float4 color).
		MTLVertexDescriptor* vertexDescriptor = [[MTLVertexDescriptor alloc] init];

		// Position attribute (Float3): location 0
		vertexDescriptor.attributes[0].format = MTLVertexFormatFloat3;
		vertexDescriptor.attributes[0].offset = 0;
		vertexDescriptor.attributes[0].bufferIndex = 0;

		// Color attribute (Float4): location 1
		vertexDescriptor.attributes[1].format = MTLVertexFormatFloat4;
		vertexDescriptor.attributes[1].offset = 12; // 3 floats * 4 bytes
		vertexDescriptor.attributes[1].bufferIndex = 0;

		// Layout for buffer 0: stride = 7 floats * 4 bytes = 28
		vertexDescriptor.layouts[0].stride = 28;
		vertexDescriptor.layouts[0].stepRate = 1;
		vertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;

		pipelineDescriptor.vertexDescriptor = vertexDescriptor;

		// Create pipeline state
		error = nil;
		id<MTLRenderPipelineState> pipelineState = [device newRenderPipelineStateWithDescriptor:pipelineDescriptor error:&error];

		if (error) {
			LR_CORE_ERROR("Failed to create Metal pipeline state: {0}",
				std::string([[error localizedDescription] UTF8String]));
			LR_CORE_ASSERT(false, "Failed to create Metal pipeline state!");
		}
		m_PipelineState = (__bridge_retained void*)pipelineState;
	}

	MetalShader::~MetalShader() {
		if (m_PipelineState) {
			CFRelease(m_PipelineState);
			m_PipelineState = nullptr;
		}
		if (m_Library) {
			CFRelease(m_Library);
			m_Library = nullptr;
		}
	}

	void MetalShader::Bind() {
		MetalContext* context = MetalContext::Get();
		if (!context || !context->GetCurrentEncoder())
			return;

		id<MTLRenderCommandEncoder> encoder = (__bridge id<MTLRenderCommandEncoder>)context->GetCurrentEncoder();
		id<MTLRenderPipelineState> pipelineState = (__bridge id<MTLRenderPipelineState>)m_PipelineState;

		// Set the pipeline state on the encoder
		[encoder setRenderPipelineState:pipelineState];

		// Upload uniforms to the vertex shader at buffer index 1
		[encoder setVertexBytes:&m_Uniforms length:sizeof(Uniforms) atIndex:1];
	}

	void MetalShader::Unbind() {
		// No-op for Metal — pipeline state is replaced by the next Bind() call
	}

	void MetalShader::SetUniform1f(const std::string& name, float value) {
		// Metal uses buffer-based uniforms — extend Uniforms struct as needed
	}

	void MetalShader::SetUniform2f(const std::string& name, const glm::vec2& vector) {
		// Metal uses buffer-based uniforms — extend Uniforms struct as needed
	}

	void MetalShader::SetUniform3f(const std::string& name, const glm::vec3& vector) {
		// Metal uses buffer-based uniforms — extend Uniforms struct as needed
	}

	void MetalShader::SetUniform4f(const std::string& name, const glm::vec4& vector) {
		// Metal uses buffer-based uniforms — extend Uniforms struct as needed
	}

	void MetalShader::SetUniformMat3f(const std::string& name, const glm::mat3& matrix) {
		// Metal uses buffer-based uniforms — extend Uniforms struct as needed
	}

	void MetalShader::SetUniformMat4f(const std::string& name, const glm::mat4& matrix) {
		if (name == "u_ViewProjection") {
			m_Uniforms.ViewProjection = matrix;
		}
		else if (name == "u_Transform") {
			m_Uniforms.Transform = matrix;
		}
	}

}

#endif // LORE_PLATFORM_MAC
