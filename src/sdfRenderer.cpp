#include "sdfRenderer.hpp"

using namespace App;


#pragma region SDFRenderer

SDFRenderer::SDFRenderer(SDL_Window* window, SDL_GPUDevice *gpu) {
	win = window;
  device = gpu;
  // create shaders
  SDL_GPUShader *vertShader = App::loadShader(device, "sdf.vert", 0, 0, 0, 0);
  SDL_GPUShader *fragShader = App::loadShader(device, "sdf.frag", 0, 1, 1, 0);
  // create pipeline
	pipeline = SDL_CreateGPUGraphicsPipeline(device, new SDL_GPUGraphicsPipelineCreateInfo {
		.vertex_shader = vertShader,
		.fragment_shader = fragShader,
		.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
		.rasterizer_state = SDL_GPURasterizerState {
			.fill_mode = SDL_GPU_FILLMODE_FILL,
			.cull_mode = SDL_GPU_CULLMODE_NONE,
		},
		.target_info = SDL_GPUGraphicsPipelineTargetInfo {
			.color_target_descriptions = new SDL_GPUColorTargetDescription {
				.format = SDL_GetGPUSwapchainTextureFormat(device, window),
			},
			.num_color_targets = 1,
		},
	});
	// create storage buffer for objects
	objsBuffer = SDL_CreateGPUBuffer(device, new SDL_GPUBufferCreateInfo {
		.usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ,
		.size = 100 * sizeof(SDFRenderObject),
	});
	// release shaders
	SDL_ReleaseGPUShader(device, vertShader);
  SDL_ReleaseGPUShader(device, fragShader);
}

void SDFRenderer::updateObjects() {
	std::vector<SDFRenderObject> objs = {
		SDFRenderObject {
			.objType = 1,
			.radius = 10.0f,
			.center = Vec2{ 200.0f, 250.0f },
			.color = Vec4{ 1.0f, 0.0f, 0.0f, 1.0f },
		},
		SDFRenderObject {
			.objType = 1,
			.radius = 40.0f,
			.center = Vec2{ 500.0f, 500.0f },
			.color = Vec4{ 0.0f, 0.0f, 1.0f, 1.0f },
		}
	};
	Uint32 objsSize = sizeof(SDFRenderObject) * objs.size();
	// update object buffer with new data
	SDL_GPUTransferBuffer *transferBuf = SDL_CreateGPUTransferBuffer(
		device,
		new SDL_GPUTransferBufferCreateInfo {
			.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
			.size = objsSize,
		}
	);
	SDFRenderObject* vertData = static_cast<SDFRenderObject*>(SDL_MapGPUTransferBuffer(
		device, transferBuf, false
	));
	for (int i=0; i < objs.size(); i++) {
		vertData[i] = objs.at(i);
	}
	SDL_UnmapGPUTransferBuffer(device, transferBuf);

	// create cmd buffer + copy pass
	SDL_GPUCommandBuffer *cmdBuf = SDL_AcquireGPUCommandBuffer(device);
	SDL_GPUCopyPass *copyPass = SDL_BeginGPUCopyPass(cmdBuf);

	SDL_UploadToGPUBuffer(
		copyPass,
		new SDL_GPUTransferBufferLocation {
			.transfer_buffer = transferBuf,
			.offset = 0,
		},
		new SDL_GPUBufferRegion {
			.buffer = objsBuffer,
			.offset = 0,
			.size = objsSize,
		},
		false
	);

	// clean up
	SDL_EndGPUCopyPass(copyPass);
	SDL_SubmitGPUCommandBuffer(cmdBuf);
	SDL_ReleaseGPUTransferBuffer(device, transferBuf);
}

int SDFRenderer::renderToScreen() {
	// acquire command buffer
	SDL_GPUCommandBuffer *cmdBuf = SDL_AcquireGPUCommandBuffer(device);

	// acquire swapchain
	SDL_GPUTexture* swapchain = NULL;
	SDL_WaitAndAcquireGPUSwapchainTexture(cmdBuf, win, &swapchain, NULL, NULL);
	if (swapchain == NULL) {
		SDL_Log("Failed to obtain swapchain: %s", SDL_GetError());
		SDL_SubmitGPUCommandBuffer(cmdBuf);
		return 1;
	}

	// define render pass
	SDL_GPURenderPass *pass = SDL_BeginGPURenderPass(cmdBuf, new SDL_GPUColorTargetInfo {
		.texture = swapchain,
		.clear_color = SDL_FColor{ 0.04f, 0.04f, 0.08f, 1.0f },
		.load_op = SDL_GPU_LOADOP_CLEAR,
		.store_op = SDL_GPU_STOREOP_STORE,
	}, 1, NULL);

	// bind pipeline to render
	SDL_BindGPUGraphicsPipeline(pass, pipeline);
	// SDL_PushGPUFragmentUniformData(cmdBuf, 0, NULL, 0);
	SDL_BindGPUFragmentStorageBuffers(pass, 0, &objsBuffer, 1);
	SDL_DrawGPUPrimitives(pass, 6, 1, 0, 0);

	// finish render pass
	SDL_EndGPURenderPass(pass);
	if (!SDL_SubmitGPUCommandBuffer(cmdBuf)) {
		SDL_Log("Failed to submit GPU command %s", SDL_GetError());
		return 2;
	};

	return 0;
}

void SDFRenderer::destroy() {
	SDL_ReleaseGPUBuffer(device, objsBuffer);
  SDL_ReleaseGPUGraphicsPipeline(device, pipeline);
}

#pragma endregion SDFRenderer
