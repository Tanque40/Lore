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
		// Store the compute pipeline in MetalContext so DispatchCompute can access it
		MetalContext* context = MetalContext::Get();
		if (context) {
			context->SetCurrentComputePipeline(m_ComputePipelineState);
		}
	}

	void MetalComputeShader::Unbind() {
		MetalContext* context = MetalContext::Get();
		if (context) {
			context->SetCurrentComputePipeline(nullptr);
		}
	}

}

#endif // LORE_PLATFORM_MAC
