#include "util.hpp"

using namespace App;

// utility function from https://github.com/TheSpydog/SDL_gpu_examples/blob/main/Examples/Common.c
SDL_GPUShader* App::loadShader(
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

Mat4x4::Mat4x4(
  float i0, float i1, float i2, float i3,
  float i4, float i5, float i6, float i7,
  float i8, float i9, float iA, float iB,
  float iC, float iD, float iE, float iF
) {
  e00 = i0; e01 = i1; e02 = i2; e03 = i3;
  e10 = i4; e11 = i5; e12 = i6; e13 = i7;
  e20 = i8; e21 = i9; e22 = iA; e23 = iB;
  e30 = iC; e31 = iD; e32 = iE; e33 = iF;
}

float* Mat4x4::rowMajor() {
  static float rm[16] = {
    e00, e01, e02, e03,
    e10, e11, e12, e13,
    e20, e21, e22, e23,
    e30, e31, e32, e33,
  };
  return rm;
}

float* Mat4x4::colMajor() {
  static float cm[16] = {
    e00, e10, e20, e30,
    e01, e11, e21, e31,
    e02, e12, e22, e32,
    e03, e13, e23, e33,
  };
  return cm;
}

Mat4x4 App::translationMat4(float x, float y, float z) {
  return Mat4x4 {
    1.0f, 0.0f, 0.0f, x,
    0.0f, 1.0f, 0.0f, y,
    0.0f, 0.0f, 1.0f, z,
    0.0f, 0.0f, 0.0f, 1.0f
  };
}

Mat4x4 App::scaleMat4(float x, float y, float z) {
  return Mat4x4 {
    x, 0.0f, 0.0f, 0.0f,
    0.0f, y, 0.0f, 0.0f,
    0.0f, 0.0f, z, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
  };
}