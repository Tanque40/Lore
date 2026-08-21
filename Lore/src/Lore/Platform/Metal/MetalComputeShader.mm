#include "lrpch.h"

#ifdef LORE_PLATFORM_MAC

#include "MetalComputeShader.h"
#include "MetalContext.h"

#import <Metal/Metal.h>

namespace Lore {

	MetalComputeShader::MetalComputeShader(const std::string& computePath) {
		MetalContext* context = MetalContext::Get();
		LR_CORE_ASSERT(context, "No active Metal context!");

		id<MTLDevice> device = (__bridge id<MTLDevice>)context->GetDevice();

		std::string computeSource = ReadFile(computePath);

		NSString* source = [NSString stringWithUTF8String:computeSource.c_str()];
		NSError* error = nil;

		MTLCompileOptions* options = [[MTLCompileOptions alloc] init];
		id<MTLLibrary> library = [device newLibraryWithSource:source options:options error:&error];

		if (error) {
			LR_CORE_ERROR("Metal compute shader compilation failed: {0}",
				std::string([[error localizedDescription] UTF8String]));
			LR_CORE_ASSERT(false, "Metal compute shader compilation failed!");
			return;
		}
		m_Library = (__bridge_retained void*)library;

		id<MTLFunction> computeFunction = [library newFunctionWithName:@"compute_main"];
		LR_CORE_ASSERT(computeFunction, "Failed to find compute_main function in Metal compute shader!");

		error = nil;
		id<MTLComputePipelineState> pipelineState = [device newComputePipelineStateWithFunction:computeFunction error:&error];

		if (error) {
			LR_CORE_ERROR("Failed to create Metal compute pipeline state: {0}",
				std::string([[error localizedDescription] UTF8String]));
			LR_CORE_ASSERT(false, "Failed to create Metal compute pipeline state!");
			return;
		}
		m_ComputePipelineState = (__bridge_retained void*)pipelineState;

		LR_CORE_INFO("Metal compute shader loaded successfully from: {0}", computePath);
	}

	MetalComputeShader::~MetalComputeShader() {
		if (m_ComputePipelineState) {
			CFRelease(m_ComputePipelineState);
			m_ComputePipelineState = nullptr;
		}
		if (m_Library) {
			CFRelease(m_Library);
			m_Library = nullptr;
		}
	}

	void MetalComputeShader::Bind() {
		// Store the compute pipeline and uniforms in MetalContext so DispatchCompute can access them
		MetalContext* context = MetalContext::Get();
		if (context) {
			context->SetCurrentComputePipeline(m_ComputePipelineState);
			context->SetComputeUniforms(&m_Uniforms, sizeof(Uniforms));
		}
	}

	void MetalComputeShader::Unbind() {
		MetalContext* context = MetalContext::Get();
		if (context) {
			context->SetCurrentComputePipeline(nullptr);
		}
	}

	void MetalComputeShader::SetUniform1f(const std::string& name, float value) {
		if (name == "u_Fov") {
			m_Uniforms.fov = value;
		} else if (name == "u_WorldSize") {
			m_Uniforms.worldSize = value;
		} else if (name == "u_MaxLevels") {
			m_Uniforms.maxLevels = value;
		} else if (name == "u_VoxelScale") {
			m_Uniforms.voxelScale = value;
		} else if (name == "u_LodPixelThreshold") {
			m_Uniforms.lodPixelThreshold = value;
		} else if (name == "u_LodCoverageThreshold") {
			m_Uniforms.lodCoverageThreshold = value;
		}
	}

	void MetalComputeShader::SetUniform2f(const std::string& name, const glm::vec2& vector){

	}

	void MetalComputeShader::SetUniform3f(const std::string& name, const glm::vec3& vector) {
		if (name == "u_CameraPos") {
			m_Uniforms.cameraPos = vector;
		} else if (name == "u_CameraDir") {
			m_Uniforms.cameraDir = vector;
		} else if (name == "u_CameraUp") {
			m_Uniforms.cameraUp = vector;
		} else if (name == "u_CameraRight") {
			m_Uniforms.cameraRight = vector;
		}
	}

	void MetalComputeShader::SetUniform4f(const std::string& name, const glm::vec4& vector){

	}

	void MetalComputeShader::SetUniformMat3f(const std::string& name, const glm::mat3& matrix) {

	}


	void MetalComputeShader::SetUniformMat4f(const std::string& name, const glm::mat4& matrix) {

	}

}

#endif // LORE_PLATFORM_MAC
