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

#pragma region Primitives

Primitive App::rect2d(float w, float h, float z) {
	w = w / 2.0f;
  h = h / 2.0f;
	std::vector<RenderVertex> vertices;
	vertices.push_back(RenderVertex{ {-w, h, z}, {0.0f, 0.0f}, {0.0f, 1.0f, 1.0f} });
	vertices.push_back(RenderVertex{ { w, h, z}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f} });
	vertices.push_back(RenderVertex{ { w,-h, z}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f} });
	vertices.push_back(RenderVertex{ {-w,-h, z}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f} });
	std::vector<Uint16> indices = { 0, 1, 2, 0, 2, 3 };
	return Primitive { vertices, indices, true };
}

Primitive App::regPolygon2d(float r, Uint16 sides, float z) {
	std::vector<RenderVertex> vertices;
	std::vector<Uint16> indices;
	float da = 2.0f * SDL_PI_F / (float)sides;
	float x0 = 1.0f; float y0 = 0.0f;
	for (int i=0; i<sides; i++) {
		float x1 = SDL_cosf(da) * x0 - SDL_sinf(da) * y0;
		float y1 = SDL_cosf(da) * y0 + SDL_sinf(da) * x0;
		// build slice
		RenderVertex p1 = RenderVertex {
			{x0 * r, y0 * r, z},
			{(1.0f + x0)/2.0f, 1.0f - (1.0f + y0)/2.0f},
			{0.0f, 0.0f, 1.0f}
		};
		RenderVertex p2 = RenderVertex {
			{x1 * r, y1 * r, z},
			{(1.0 + x1)/2.0, 1.0 - (1.0 + y1)/2.0},
			{0.0f, 0.0f, 1.0f}
		};
		RenderVertex p3 = RenderVertex {
			{0.0f, 0.0f, z}, {0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}
		};
		// prepare next slice
		x0 = x1;
		y0 = y1;
		vertices.push_back(p1);
		vertices.push_back(p2);
		vertices.push_back(p3);
	}
	return Primitive { vertices, indices, false };
}

Primitive App::torus2d(float outerRadius, float innerRadius, Uint16 sides, float z) {
	std::vector<RenderVertex> vertices;
	std::vector<Uint16> indices;
	float dr = innerRadius / outerRadius;
	// build vertices
	for (int i=0; i<sides; i++) {
		float theta = 2.0f * SDL_PI_F * ((float)i / (float)sides);
		float x = SDL_cosf(theta);
		float y = SDL_sinf(theta);
		RenderVertex v1 {
			{x * outerRadius, y * outerRadius, z},
			{(1.0f + x)/2.0f, (1.0f + y)/2.0f},
			{0.0f, 0.0f, 1.0f}
		};
		RenderVertex v2 {
			{x * innerRadius, y * innerRadius, z},
			{(1.0f + dr * x) / 2.0f, (1.0f + dr * y) / 2.0f},
			{0.0f, 0.0f, 1.0f}
		};
		vertices.push_back(v1);
		vertices.push_back(v2);
	}
	// build indices
	for (int i=0; i<vertices.size() - 2; i++) {
		if (i % 2 == 0) {
			indices.push_back(i + 1);
			indices.push_back(i);
			indices.push_back(i + 2);
		} else {
			indices.push_back(i);
			indices.push_back(i + 1);
			indices.push_back(i + 2);
		}
	}
	// join back to first 2 vertices
	indices.push_back(vertices.size() - 1);
	indices.push_back(vertices.size() - 2);
	indices.push_back(0);
	indices.push_back(vertices.size() - 1);
	indices.push_back(0);
	indices.push_back(1);
	return Primitive { vertices, indices, true };
}

Primitive App::sphere(float r, Uint16 sides, Uint16 slices) {
	std::vector<RenderVertex> vertices;
	std::vector<Uint16> indices;

	return Primitive { vertices, indices, true };
}

Primitive App::cube(float w, float h, float d) {
	std::vector<RenderVertex> vertices;
	w = w / 2.0f; h = h / 2.0f; d = d / 2.0f;
	// front
	vertices.push_back(RenderVertex{ {-w,-h,-d}, {0.0f, 0.0f}, {0.0f, 0.0f,-1.0f} });
	vertices.push_back(RenderVertex{ {-w, h,-d}, {1.0f, 0.0f}, {0.0f, 0.0f,-1.0f} });
	vertices.push_back(RenderVertex{ { w, h,-d}, {1.0f, 1.0f}, {0.0f, 0.0f,-1.0f} });
	vertices.push_back(RenderVertex{ { w,-h,-d}, {0.0f, 1.0f}, {0.0f, 0.0f,-1.0f} });
	// back
	vertices.push_back(RenderVertex{ {-w,-h, d}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f} });
	vertices.push_back(RenderVertex{ {-w, h, d}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f} });
	vertices.push_back(RenderVertex{ { w, h, d}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f} });
	vertices.push_back(RenderVertex{ { w,-h, d}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f} });
	// top
	vertices.push_back(RenderVertex{ {-w,-h,-d}, {0.0f, 1.0f}, {0.0f,-1.0f, 0.0f} });
	vertices.push_back(RenderVertex{ { w,-h,-d}, {1.0f, 1.0f}, {0.0f,-1.0f, 0.0f} });
	vertices.push_back(RenderVertex{ {-w,-h, d}, {0.0f, 0.0f}, {0.0f,-1.0f, 0.0f} });
	vertices.push_back(RenderVertex{ { w,-h, d}, {1.0f, 0.0f}, {0.0f,-1.0f, 0.0f} });
	// bottom
	vertices.push_back(RenderVertex{ {-w, h,-d}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f} });
	vertices.push_back(RenderVertex{ { w, h,-d}, {0.0f, 1.0f}, {0.0f, 1.0f, 0.0f} });
	vertices.push_back(RenderVertex{ {-w, h, d}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f} });
	vertices.push_back(RenderVertex{ { w, h, d}, {1.0f, 1.0f}, {0.0f, 1.0f, 0.0f} });
	// left
	vertices.push_back(RenderVertex{ {-w,-h,-d}, {1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f} });
	vertices.push_back(RenderVertex{ {-w, h,-d}, {1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f} });
	vertices.push_back(RenderVertex{ {-w,-h, d}, {0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f} });
	vertices.push_back(RenderVertex{ {-w, h, d}, {0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f} });
	// right
	vertices.push_back(RenderVertex{ { w,-h,-d}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f} });
	vertices.push_back(RenderVertex{ { w, h,-d}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f} });
	vertices.push_back(RenderVertex{ { w,-h, d}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f} });
	vertices.push_back(RenderVertex{ { w, h, d}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f} });
	
	std::vector<Uint16> indices = {
		0,1,2, 0,2,3,
		4,6,5, 4,7,6,
		8,9,10, 9,11,10,
		12,14,13, 14,15,13,
		18,19,16, 19,17,16,
		20,21,22, 21,23,22
	};
	return Primitive { vertices, indices, true };
}

Primitive App::cylinder(float r, float h, Uint16 sides) {
	std::vector<RenderVertex> vertices;
	std::vector<Uint16> indices;

	return Primitive { vertices, indices, true };
}

#pragma endregion Primitives