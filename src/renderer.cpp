#include "app.hpp"

using namespace App;

// utility function from https://github.com/TheSpydog/SDL_gpu_examples/blob/main/Examples/Common.c
SDL_GPUShader* loadShader(
  SDL_GPUDevice *device,
  const char* filename,
  Uint32 samplerCount,
  Uint32 uniformBufferCount,
	Uint32 storageBufferCount,
	Uint32 storageTextureCount
) {
  // Auto-detect the shader stage from the file name for convenience
	SDL_GPUShaderStage stage;
	if (SDL_strstr(filename, ".vert")) {
		stage = SDL_GPU_SHADERSTAGE_VERTEX;
	} else if (SDL_strstr(filename, ".frag")) {
		stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
	} else {
		SDL_Log("Invalid shader stage!");
		return NULL;
	}

  char fullPath[256];
	SDL_GPUShaderFormat backendFormats = SDL_GetGPUShaderFormats(device);
	SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_INVALID;
	const char *entrypoint;

  if (backendFormats & SDL_GPU_SHADERFORMAT_SPIRV) {
		SDL_snprintf(fullPath, sizeof(fullPath), "assets/SPIRV/%s.spv", filename);
		format = SDL_GPU_SHADERFORMAT_SPIRV;
		entrypoint = "main";
	} else if (backendFormats & SDL_GPU_SHADERFORMAT_MSL) {
		SDL_snprintf(fullPath, sizeof(fullPath), "assets/MSL/%s.msl", filename);
		format = SDL_GPU_SHADERFORMAT_MSL;
		entrypoint = "main0";
	} else if (backendFormats & SDL_GPU_SHADERFORMAT_DXIL) {
		SDL_snprintf(fullPath, sizeof(fullPath), "assets/DXIL/%s.dxil", filename);
		format = SDL_GPU_SHADERFORMAT_DXIL;
		entrypoint = "main";
	} else {
		SDL_Log("%s", "Unrecognized backend shader format!");
		return NULL;
	}

  size_t codeSize;
	void* code = SDL_LoadFile(fullPath, &codeSize);
	if (code == NULL) {
		SDL_Log("Failed to load shader from disk! %s", fullPath);
		return NULL;
	}

	SDL_GPUShaderCreateInfo shaderInfo = {
    .code_size = codeSize,
		.code = static_cast<Uint8*>(code),
		.entrypoint = entrypoint,
		.format = format,
		.stage = stage,
		.num_samplers = samplerCount,
    .num_storage_textures = storageTextureCount,
		.num_storage_buffers = storageBufferCount,
    .num_uniform_buffers = uniformBufferCount
	};
  SDL_GPUShader* shader = SDL_CreateGPUShader(device, &shaderInfo);
	if (shader == NULL) {
		SDL_Log("Failed to create shader!");
		SDL_free(code);
		return NULL;
	}

	SDL_free(code);
  return shader;
}

// create vertex input state corresponding to RenderVertex shape
SDL_GPUVertexInputState createVertexInputState() {
	SDL_GPUVertexInputState state;

	state.vertex_buffer_descriptions = new SDL_GPUVertexBufferDescription {
		.slot = 0,
		.pitch = sizeof(RenderVertex),
		.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
		.instance_step_rate = 0,
	};
	state.num_vertex_buffers = 1;
	
	SDL_GPUVertexAttribute vAttr0 = {
		.location = 0,
		.buffer_slot = 0,
		.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
		.offset = 0,
	};
	SDL_GPUVertexAttribute vAttr1 = {
		.location = 1,
		.buffer_slot = 0,
		.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
		.offset = sizeof(float) * 3,
	};
	SDL_GPUVertexAttribute vAttr2 = {
		.location = 2,
		.buffer_slot = 0,
		.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
		.offset = sizeof(float) * 5,
	};
	state.vertex_attributes = new SDL_GPUVertexAttribute[3] {
		vAttr0, vAttr1, vAttr2
	};
	state.num_vertex_attributes = 3;

	return state;
}

// helper for creating pipeline
SDL_GPUGraphicsPipeline* createPipeline(
	SDL_Window *window,
	SDL_GPUDevice *device,
	SDL_GPUShader *vertShader,
	SDL_GPUShader *fragShader,
	GPUPrimitiveType primitiveType
) {
	SDL_GPUGraphicsPipelineCreateInfo createInfo{};
	createInfo.vertex_shader = vertShader;
	createInfo.fragment_shader = fragShader;

	createInfo.target_info = SDL_GPUGraphicsPipelineTargetInfo {
		.color_target_descriptions = new SDL_GPUColorTargetDescription {
			.format = SDL_GetGPUSwapchainTextureFormat(device, window),
		},
		.num_color_targets = 1,
	};

	createInfo.vertex_input_state = createVertexInputState();
	
	switch (primitiveType) {
		case GPUPrimitiveType::Point:
			createInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_POINTLIST;
			break;
		case GPUPrimitiveType::Line:
			createInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_LINELIST;
			createInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
			break;
		default:
			createInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
			createInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
			break;
	}

	return SDL_CreateGPUGraphicsPipeline(device, &createInfo);
}

#pragma region RenderInstance

RenderInstance::RenderInstance(SDL_Window *window, SDL_GPUDevice *gpu) {
  win = window;
  device = gpu;
  // create shaders
  SDL_GPUShader *vertShader = loadShader(device, "test.vert", 0, 0, 0, 0);
  SDL_GPUShader *fragShader = loadShader(device, "test.frag", 0, 0, 0, 0);
  // create pipeline
  pipeline = createPipeline(win, device, vertShader, fragShader, GPUPrimitiveType::Triangle);

	// create vertices
	std::vector<RenderVertex> vertices;
	// x, y, z, u, v, nx, ny, nz
	vertices.push_back(RenderVertex{ -0.8f,  0.8f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f });
	vertices.push_back(RenderVertex{  0.8f,  0.8f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f });
	vertices.push_back(RenderVertex{  0.8f, -0.8f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f });
	vertices.push_back(RenderVertex{ -0.8f, -0.8f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f });
	std::vector<Uint16> indices = { 0, 1, 2, 0, 2, 3 };
	uploadVertices(vertices, indices);

	// release shaders
	SDL_ReleaseGPUShader(device, vertShader);
  SDL_ReleaseGPUShader(device, fragShader);
}

void RenderInstance::uploadVertices(std::vector<RenderVertex> verts) {
	std::vector<Uint16> none;
	uploadVertices(verts, none);
}

void RenderInstance::uploadVertices(std::vector<RenderVertex> verts, std::vector<Uint16> indices) {
	// create vertex buffer
	SDL_ReleaseGPUBuffer(device, vertexBuffer);
	Uint32 vSize = sizeof(RenderVertex) * verts.size();
	vertexBuffer = SDL_CreateGPUBuffer(device, new SDL_GPUBufferCreateInfo {
		.usage = SDL_GPU_BUFFERUSAGE_VERTEX,
		.size = vSize
	});

	// create index buffer
	SDL_ReleaseGPUBuffer(device, indexBuffer);
	Uint32 iSize = sizeof(Uint16) * indices.size();
	indexBuffer = SDL_CreateGPUBuffer(device, new SDL_GPUBufferCreateInfo{
		.usage = SDL_GPU_BUFFERUSAGE_INDEX,
		.size = iSize
	});

	// pump vertex data into transfer buffer
	SDL_GPUTransferBuffer *vertTransferBuf = SDL_CreateGPUTransferBuffer(
		device,
		new SDL_GPUTransferBufferCreateInfo {
			.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
			.size = vSize,
		}
	);
	RenderVertex* vertData = static_cast<RenderVertex*>(SDL_MapGPUTransferBuffer(
		device, vertTransferBuf, false
	));
	for (int i=0; i < verts.size(); i++) {
		vertData[i] = verts.at(i);
	}
	SDL_UnmapGPUTransferBuffer(device, vertTransferBuf);

	// pump index data into transfer buffer
	SDL_GPUTransferBuffer *idxTransferBuf = SDL_CreateGPUTransferBuffer(
		device,
		new SDL_GPUTransferBufferCreateInfo {
			.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
			.size = iSize,
		}
	);
	Uint16* indexData = static_cast<Uint16*>(SDL_MapGPUTransferBuffer(
		device, idxTransferBuf, false
	));
	for (int i=0; i < indices.size(); i++) {
		indexData[i] = indices.at(i);
	}
	SDL_UnmapGPUTransferBuffer(device, idxTransferBuf);

	// create cmd buffer + copy pass
	SDL_GPUCommandBuffer *cmdBuf = SDL_AcquireGPUCommandBuffer(device);
	SDL_GPUCopyPass *copyPass = SDL_BeginGPUCopyPass(cmdBuf);

	SDL_UploadToGPUBuffer(
		copyPass,
		new SDL_GPUTransferBufferLocation {
			.transfer_buffer = vertTransferBuf,
			.offset = 0,
		},
		new SDL_GPUBufferRegion {
			.buffer = vertexBuffer,
			.offset = 0,
			.size = vSize,
		},
		false
	);

	SDL_UploadToGPUBuffer(
		copyPass,
		new SDL_GPUTransferBufferLocation {
			.transfer_buffer = idxTransferBuf,
			.offset = 0,
		},
		new SDL_GPUBufferRegion {
			.buffer = indexBuffer,
			.offset = 0,
			.size = iSize,
		},
		false
	);

	// clean up passes
	SDL_EndGPUCopyPass(copyPass);
	SDL_SubmitGPUCommandBuffer(cmdBuf);
	// clean up transfer buffers
	SDL_ReleaseGPUTransferBuffer(device, vertTransferBuf);
	SDL_ReleaseGPUTransferBuffer(device, idxTransferBuf);
}

int RenderInstance::renderToScreen() {
  // todo: render steps
  // 1. SDL_AcquireGPUCommandBuffer
	SDL_GPUCommandBuffer *cmdBuf = SDL_AcquireGPUCommandBuffer(device);
  // 2. SDL_WaitAndAcquireGPUSwapchainTexture -> render to window
	SDL_GPUTexture* swapchain = NULL;
	SDL_WaitAndAcquireGPUSwapchainTexture(cmdBuf, win, &swapchain, NULL, NULL);
	if (swapchain == NULL) {
		SDL_Log("Failed to obtain swapchain: %s", SDL_GetError());
		SDL_SubmitGPUCommandBuffer(cmdBuf);
		return 1;
	}
  // 3. SDL_BeginGPURenderPass
	SDL_GPUColorTargetInfo colorTarget{ 0 };
	colorTarget.texture = swapchain;
	colorTarget.store_op = SDL_GPU_STOREOP_STORE;
	colorTarget.load_op = SDL_GPU_LOADOP_CLEAR;
	colorTarget.clear_color = SDL_FColor{ 0.04f, 0.04f, 0.08f, 1.0f };
	SDL_GPURenderPass *pass = SDL_BeginGPURenderPass(cmdBuf, &colorTarget, 1, NULL);

  // bind pipeline to render
	SDL_BindGPUGraphicsPipeline(pass, pipeline);
	// SDL_PushGPUVertexUniformData();
	SDL_BindGPUVertexBuffers(pass, 0, new SDL_GPUBufferBinding{vertexBuffer, 0 }, 1);
	// 4. Draw to screen
	if (indexBuffer != NULL) {
		SDL_BindGPUIndexBuffer(pass, new SDL_GPUBufferBinding {indexBuffer, 0 }, SDL_GPU_INDEXELEMENTSIZE_16BIT);
		SDL_DrawGPUIndexedPrimitives(pass, 6, 1, 0, 0, 0);
	} else {
		SDL_DrawGPUPrimitives(pass, 3, 1, 0, 0);
	}

	// finish render pass
	SDL_EndGPURenderPass(pass);
	if (!SDL_SubmitGPUCommandBuffer(cmdBuf)) {
		SDL_Log("Failed to submit GPU command %s", SDL_GetError());
		return 2;
	};

	return 0;
}

void RenderInstance::destroy() {
	SDL_ReleaseGPUBuffer(device, vertexBuffer);
	SDL_ReleaseGPUBuffer(device, indexBuffer);
  SDL_ReleaseGPUGraphicsPipeline(device, pipeline);
}

#pragma endregion RenderInstance