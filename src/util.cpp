#include "util.hpp"

using namespace App;

#pragma region Pipeline helpers

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

// create vertex input state corresponding to RenderVertex shape
SDL_GPUVertexInputState App::createVertexInputState() {
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
	SDL_GPUVertexAttribute vAttr3 = {
		.location = 3,
		.buffer_slot = 0,
		.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
		.offset = sizeof(float) * 8,
	};
	state.vertex_attributes = new SDL_GPUVertexAttribute[4] {
		vAttr0, vAttr1, vAttr2, vAttr3
	};
	state.num_vertex_attributes = 4;

	return state;
}

// generic handler for copying vertex data into buffers
void App::copyVertexDataIntoBuffer(
	SDL_GPUDevice *device, SDL_GPUBuffer *vertBuf, SDL_GPUBuffer *indexBuf,
	std::vector<RenderVertex> *verts, std::vector<Uint16> *indices
) {
	Uint32 vSize = sizeof(RenderVertex) * verts->size();
	Uint32 iSize = sizeof(Uint16) * indices->size();

	// pump vertex data into transfer buffer
	SDL_GPUTransferBuffer *vertTransferBuf = SDL_CreateGPUTransferBuffer(
		device,
		new SDL_GPUTransferBufferCreateInfo {
			.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
			.size = vSize,
		}
	);
	RenderVertex* vertData = static_cast<RenderVertex*>(SDL_MapGPUTransferBuffer(device, vertTransferBuf, false));
	for (int i=0; i < verts->size(); i++) {
		vertData[i] = verts->at(i);
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
	Uint16* indexData = static_cast<Uint16*>(SDL_MapGPUTransferBuffer(device, idxTransferBuf, false));
	for (int i=0; i < indices->size(); i++) {
		indexData[i] = indices->at(i);
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
			.buffer = vertBuf,
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
			.buffer = indexBuf,
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

#pragma endregion Pipeline helpers

#pragma region Color utils

SDL_FColor App::rgba(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	return SDL_FColor{ r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f };
}

SDL_FColor App::rgb(Uint8 r, Uint8 g, Uint8 b) {
	return rgba(r, g, b, 255);
}

SDL_FColor App::hsva(float h, float s, float v, float a) {
	float i = SDL_floorf(h * 6.0);
	float f = h * 6.0 - i;
	float p = v * (1.0 - s);
	float q = v * (1.0 - f * s);
	float t = v * (1.0 - (1.0 - f) * s);

	float fmod = SDL_fmodf(i, 6.0f);

	switch ((int)i % 6) {
		case 0:
			return SDL_FColor{v, t, p, a};
		case 1:
			return SDL_FColor{q, v, p, a};
		case 2:
			return SDL_FColor{p, v, t, a};
		case 3:
			return SDL_FColor{p, q, v, a};
		case 4:
			return SDL_FColor{t, p, v, a};
		case 5:
			return SDL_FColor{v, p, q, a};
		case 6:
			return SDL_FColor{v, p, q, a};
	}
	return WHITE;
}

SDL_FColor App::hsv(float h, float s, float v) {
	return hsva(h, s, v, 1.0f);
}

SDL_FColor App::modAlpha(SDL_FColor clr, float a) {
	return SDL_FColor{ clr.r, clr.g, clr.b, a };
}

#pragma endregion Color utils

#pragma region Linear algebra

float Vec2::magnitude() {
	return SDL_sqrtf(x*x + y*y);
}

Vec2 Vec2::normalize() {
	float v = magnitude();
	return Vec2{x/v, y/v};
}

float Vec3::magnitude() {
	return SDL_sqrtf(x*x + y*y + z*z);
}

Vec3 Vec3::normalize() {
	float v = magnitude();
	return Vec3{x/v, y/v, z/v};
}

float Vec4::magnitude() {
	return SDL_sqrtf(x*x + y*y + z*z + w*w);
}

Vec4 Vec4::normalize() {
	float v = magnitude();
	return Vec4{x/v, y/v, z/v, w/v};
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

float App::dot(Vec2 a, Vec2 b) {
	return a.x * b.x + a.y * b.y;
}

float App::dot(Vec3 a, Vec3 b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

float App::dot(Vec4 a, Vec4 b) {
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

Vec4 App::multiplyMat4(Mat4x4 a, Vec4 b) {
	return Vec4 {
		b.x * a.e00 + b.y * a.e01 + b.z * a.e02 + b.w * a.e03,
		b.x * a.e10 + b.y * a.e11 + b.z * a.e12 + b.w * a.e13,
		b.x * a.e20 + b.y * a.e21 + b.z * a.e22 + b.w * a.e23,
		b.x * a.e30 + b.y * a.e31 + b.z * a.e32 + b.w * a.e33,
	};
}

Mat4x4 App::multiplyMat4(Mat4x4 a, Mat4x4 b) {
	return Mat4x4(
		a.e00 * b.e00 + a.e10 * b.e01 + a.e20 * b.e02 + a.e30 * b.e03,
		a.e00 * b.e10 + a.e10 * b.e11 + a.e20 * b.e12 + a.e30 * b.e13,
		a.e00 * b.e20 + a.e10 * b.e21 + a.e20 * b.e22 + a.e30 * b.e23,
		a.e00 * b.e30 + a.e10 * b.e31 + a.e20 * b.e32 + a.e30 * b.e33,

		a.e01 * b.e00 + a.e11 * b.e01 + a.e21 * b.e02 + a.e31 * b.e03,
    a.e01 * b.e10 + a.e11 * b.e11 + a.e21 * b.e12 + a.e31 * b.e13,
    a.e01 * b.e20 + a.e11 * b.e21 + a.e21 * b.e22 + a.e31 * b.e23,
    a.e01 * b.e30 + a.e11 * b.e31 + a.e21 * b.e32 + a.e31 * b.e33,

		a.e02 * b.e00 + a.e12 * b.e01 + a.e22 * b.e02 + a.e32 * b.e03,
		a.e02 * b.e10 + a.e12 * b.e11 + a.e22 * b.e12 + a.e32 * b.e13,
		a.e02 * b.e20 + a.e12 * b.e21 + a.e22 * b.e22 + a.e32 * b.e23,
		a.e02 * b.e30 + a.e12 * b.e31 + a.e22 * b.e32 + a.e32 * b.e33,

		a.e03 * b.e00 + a.e13 * b.e01 + a.e23 * b.e02 + a.e33 * b.e03,
		a.e03 * b.e10 + a.e13 * b.e11 + a.e23 * b.e12 + a.e33 * b.e13,
		a.e03 * b.e20 + a.e13 * b.e21 + a.e23 * b.e22 + a.e33 * b.e23,
		a.e03 * b.e30 + a.e13 * b.e31 + a.e23 * b.e32 + a.e33 * b.e33
	);
}

Mat4x4 App::identityMat4() {
	return Mat4x4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
}

Mat4x4 App::perspectiveMat4(float fovY, float aspectRatio, float near, float far) {
	float f = SDL_tanf(SDL_PI_F * 0.5f - 0.5f * fovY * SDL_PI_F / 180.0f);
	float range = 1.0f / (near - far);
	float a = f / aspectRatio;
	float c = far * range;
	float d = near * far * range;
	return Mat4x4(
		a, 0.0f, 0.0f, 0.0f,
		0.0f, f, 0.0f, 0.0f,
		0.0f, 0.0f, c, d,
		0.0f, 0.0f, -1.0f, 0.0f
	);
}

Mat4x4 App::orthoMat4(float left, float right, float top, float bottom, float near, float far) {
	float a = 2.0f / (right - left);
	float b = 2.0f / (top - bottom);
	float c = 1.0f / (near - far);
	float d = (right + left) / (left - right);
	float e = (top + bottom) / (bottom - top);
	float f = near / (near - far);
	return Mat4x4(
		a, 0.0f, 0.0f, d,
		0.0f, b, 0.0f, e,
		0.0f, 0.0f, c, f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
}

Mat4x4 App::translationMat4(float x, float y, float z) {
  return Mat4x4 {
    1.0f, 0.0f, 0.0f, x,
    0.0f, 1.0f, 0.0f, y,
    0.0f, 0.0f, 1.0f, z,
    0.0f, 0.0f, 0.0f, 1.0f
  };
}

Mat4x4 App::rotationMat4(Vec3 axis, float degree)  {
	Vec3 n = axis.normalize();

	return identityMat4();
}

Mat4x4 App::rotationEulerMat4(float roll, float pitch, float yaw) {
	float a = roll * SDL_PI_F / 180.0f;
	float cosa = SDL_cosf(a);
	float sina = SDL_cosf(a);
	float b = pitch * SDL_PI_F / 180.0f;
	float cosb = SDL_cosf(b);
	float sinb = SDL_cosf(b);
	float c = yaw * SDL_PI_F / 180.0f;
	float cosc = SDL_cosf(c);
	float sinc = SDL_cosf(c);
	Mat4x4 rx = Mat4x4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, cosa, -sina, 0.0f,
		0.0f, sina, cosa, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
	Mat4x4 ry = Mat4x4(
		cosb, 0.0f, sinb, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		-sinb, 0.0f, cosb, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
	Mat4x4 rz = Mat4x4(
		cosc, -sinc, 0.0f, 0.0f,
		sinc, cosc, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
	return multiplyMat4(multiplyMat4(rx, ry), rz);
}

Mat4x4 App::scaleMat4(float x, float y, float z) {
  return Mat4x4 {
    x, 0.0f, 0.0f, 0.0f,
    0.0f, y, 0.0f, 0.0f,
    0.0f, 0.0f, z, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
  };
}

#pragma endregion Linear algebra