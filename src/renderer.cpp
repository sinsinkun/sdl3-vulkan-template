#include "app.hpp"

using namespace App;

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
		format = SDL_GPU_SHADERFORMAT_SPIRV;
		entrypoint = "main";
	} else if (backendFormats & SDL_GPU_SHADERFORMAT_MSL) {
		format = SDL_GPU_SHADERFORMAT_MSL;
		entrypoint = "main0";
	} else if (backendFormats & SDL_GPU_SHADERFORMAT_DXIL) {
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

SDL_GPUGraphicsPipeline* createPipeline(
	SDL_GPUDevice *device, SDL_GPUShader *vertShader, SDL_GPUShader *fragShader
) {
	SDL_GPUGraphicsPipelineCreateInfo createInfo{};
	createInfo.vertex_shader = vertShader;
	createInfo.fragment_shader = fragShader;
	return SDL_CreateGPUGraphicsPipeline(device, &createInfo);
}

RenderInstance::RenderInstance(SDL_Window *window, SDL_GPUDevice *gpu) {
  win = window;
  device = gpu;
  // create shaders
  vertShader = loadShader(device, "shaders/test.vert", 0, 0, 0, 0);
  fragShader = loadShader(device, "shaders/test.frag", 0, 0, 0, 0);
  // create pipeline
  // pipeline = createPipeline(device, vertShader, fragShader);
}

void RenderInstance::renderToTexture() {
  // todo: render steps
}

void RenderInstance::renderToScreen() {
  // todo: render steps
  // 1. SDL_AcquireGPUCommandBuffer
	SDL_GPUCommandBuffer *cmdBuf = SDL_AcquireGPUCommandBuffer(device);
  // 2. SDL_WaitAndAcquireGPUSwapchainTexture -> render to window
	SDL_GPUTexture* swapchain = NULL;
	SDL_WaitAndAcquireGPUSwapchainTexture(cmdBuf, win, &swapchain, NULL, NULL);
	if (swapchain == NULL) {
		SDL_Log("Failed to obtain swapchain");
		SDL_SubmitGPUCommandBuffer(cmdBuf);
		return;
	}
  // 3. SDL_BeginGPURenderPass
	SDL_GPUColorTargetInfo colorTarget{};
	colorTarget.texture = swapchain;
	colorTarget.store_op = SDL_GPU_STOREOP_STORE;
	colorTarget.load_op = SDL_GPU_LOADOP_CLEAR;
	colorTarget.clear_color = SDL_FColor{ 0.08f, 0.02f, 0.2f, 1.0f };
	std::vector <SDL_GPUColorTargetInfo> colorTargets{colorTarget};
	SDL_GPURenderPass *pass = SDL_BeginGPURenderPass(cmdBuf, colorTargets.data(), colorTargets.size(), NULL);
  // 3.1. SDL_BindGPUGraphicsPipeline
  // 3.2. SDL_SetGPUViewport
  // 3.3. SDL_BindGPUVertexBuffer
  // 3.4. SDL_BindGPUVertexSamplers
  // 4. SDL_DrawGPUPrimitives
  // 4.1 SDL_DrawGPUPrimitivesIndirect
  // 4.2 SDL_DrawGPUIndexedPrimitivesIndirect
  // 5. SDL_EndGPURenderPass
	SDL_EndGPURenderPass(pass);
  // 6. SDL_SubmitGPUCommandBuffer
	if (!SDL_SubmitGPUCommandBuffer(cmdBuf)) {
		SDL_Log("Failed to submit GPU command %s", SDL_GetError());
	};
}

void RenderInstance::destroy() {
  SDL_ReleaseGPUShader(device, vertShader);
  SDL_ReleaseGPUShader(device, fragShader);
  SDL_ReleaseGPUGraphicsPipeline(device, pipeline);
}