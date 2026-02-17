#include "lrpch.h"

#ifdef LORE_PLATFORM_MAC

#include "MetalShader.h"
#include "MetalContext.h"

#import <Metal/Metal.h>

namespace Lore {

	MetalShader::MetalShader(const std::string& vertexSrc, const std::string& fragmentSrc) {
		CreatePipelineState(vertexSrc, fragmentSrc);
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

	void MetalShader::CreatePipelineState(const std::string& vertexSrc, const std::string& fragmentSrc) {
		id<MTLDevice> device = (__bridge id<MTLDevice>)MetalContext::Get()->GetDevice();

		// Combine both shader sources (Metal uses a single library with named functions)
		// The vertexSrc should contain a function named "vertexShader"
		// The fragmentSrc should contain a function named "fragmentShader"
		std::string combinedSource = vertexSrc + "\n" + fragmentSrc;

		NSString* source = [NSString stringWithUTF8String:combinedSource.c_str()];
		NSError* error = nil;

		MTLCompileOptions* options = [[MTLCompileOptions alloc] init];
		id<MTLLibrary> library = [device newLibraryWithSource:source options:options error:&error];

		if (!library) {
			NSString* errorStr = [error localizedDescription];
			LR_CORE_ERROR("Metal shader compilation failed: {0}", [errorStr UTF8String]);
			LR_CORE_ASSERT(false, "Metal shader compilation failed!");
			return;
		}
		m_Library = (__bridge_retained void*)library;

		id<MTLFunction> vertexFunction = [library newFunctionWithName:@"vertexShader"];
		id<MTLFunction> fragmentFunction = [library newFunctionWithName:@"fragmentShader"];

		if (!vertexFunction) {
			LR_CORE_ERROR("Metal vertex function 'vertexShader' not found!");
			LR_CORE_ASSERT(false, "Metal vertex function not found!");
			return;
		}
		if (!fragmentFunction) {
			LR_CORE_ERROR("Metal fragment function 'fragmentShader' not found!");
			LR_CORE_ASSERT(false, "Metal fragment function not found!");
			return;
		}

		// Create pipeline descriptor
		MTLRenderPipelineDescriptor* pipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
		pipelineDescriptor.vertexFunction = vertexFunction;
		pipelineDescriptor.fragmentFunction = fragmentFunction;
		pipelineDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;

		// Set up vertex descriptor for position (float3)
		MTLVertexDescriptor* vertexDescriptor = [[MTLVertexDescriptor alloc] init];
		vertexDescriptor.attributes[0].format = MTLVertexFormatFloat3;
		vertexDescriptor.attributes[0].offset = 0;
		vertexDescriptor.attributes[0].bufferIndex = 0;
		vertexDescriptor.layouts[0].stride = sizeof(float) * 3;
		vertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;

		pipelineDescriptor.vertexDescriptor = vertexDescriptor;

		// Create pipeline state
		error = nil;
		id<MTLRenderPipelineState> pipelineState = [device newRenderPipelineStateWithDescriptor:pipelineDescriptor error:&error];

		if (!pipelineState) {
			NSString* errorStr = [error localizedDescription];
			LR_CORE_ERROR("Metal pipeline state creation failed: {0}", [errorStr UTF8String]);
			LR_CORE_ASSERT(false, "Metal pipeline state creation failed!");
			return;
		}

		m_PipelineState = (__bridge_retained void*)pipelineState;

		// ARC manages vertexFunction and fragmentFunction automatically
	}

	void MetalShader::Bind() const {
		if (!m_PipelineState || !MetalContext::Get() || !MetalContext::Get()->GetCurrentEncoder())
			return;

		id<MTLRenderCommandEncoder> encoder = (__bridge id<MTLRenderCommandEncoder>)MetalContext::Get()->GetCurrentEncoder();
		id<MTLRenderPipelineState> pipelineState = (__bridge id<MTLRenderPipelineState>)m_PipelineState;
		[encoder setRenderPipelineState:pipelineState];
	}

	void MetalShader::Unbind() const {
		// Metal doesn't have a concept of unbinding pipeline state
	}

}

#endif
