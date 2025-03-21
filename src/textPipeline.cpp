#include "textPipeline.hpp"

using namespace App;

StringObject::StringObject(TTF_TextEngine *textEngine, TTF_Font *font, std::string text) {
	this->text = text;
	this->ttfText = TTF_CreateText(textEngine, font, text.c_str(), text.length());
}

void StringObject::updateText(std::string text) {
	bool success = TTF_SetTextString(ttfText, text.c_str(), text.length());
	if (!success) {
		SDL_Log("Failed to set text string: %s", SDL_GetError());
		return;
	}
	success = TTF_UpdateText(ttfText);
	if (!success) {
		SDL_Log("Failed to update text string: %s", SDL_GetError());
	}
}

TextPipeline::TextPipeline(SDL_GPUTextureFormat targetFormat, SDL_GPUDevice *gpu) {
  device = gpu;
  // create shaders
  SDL_GPUShader *vertShader = App::loadShader(device, "ttfRects.vert", 0, 1, 0, 0);
  SDL_GPUShader *fragShader = App::loadShader(device, "ttfRects.frag", 1, 1, 0, 0);
  // create pipeline
	pipeline = SDL_CreateGPUGraphicsPipeline(device, new SDL_GPUGraphicsPipelineCreateInfo {
		.vertex_shader = vertShader,
		.fragment_shader = fragShader,
    .vertex_input_state = createVertexInputState(),
		.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
		.rasterizer_state = SDL_GPURasterizerState {
			.fill_mode = SDL_GPU_FILLMODE_FILL,
			.cull_mode = SDL_GPU_CULLMODE_NONE,
		},
		.target_info = SDL_GPUGraphicsPipelineTargetInfo {
			.color_target_descriptions = new SDL_GPUColorTargetDescription {
				.format = targetFormat,
				.blend_state = SDL_GPUColorTargetBlendState {
					.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
					.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
					.color_blend_op = SDL_GPU_BLENDOP_ADD,
					.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
					.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
					.alpha_blend_op = SDL_GPU_BLENDOP_ADD,
					.enable_blend = true,
				},
			},
			.num_color_targets = 1,
		},
	});
  // create sampler
  sampler = SDL_CreateGPUSampler(device, new SDL_GPUSamplerCreateInfo {
    .min_filter = SDL_GPU_FILTER_LINEAR,
    .mag_filter = SDL_GPU_FILTER_LINEAR,
    .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
    .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
    .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
    .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE
  });
  // create vertex buffer
  vertBuf = SDL_CreateGPUBuffer(device, new SDL_GPUBufferCreateInfo {
		.usage = SDL_GPU_BUFFERUSAGE_VERTEX,
		.size = sizeof(RenderVertex) * MAX_VERT_COUNT
	});
  // create index buffer
  indexBuf = SDL_CreateGPUBuffer(device, new SDL_GPUBufferCreateInfo {
		.usage = SDL_GPU_BUFFERUSAGE_INDEX,
		.size = sizeof(Uint16) * MAX_INDEX_COUNT
	});

  // release shaders
	SDL_ReleaseGPUShader(device, vertShader);
  SDL_ReleaseGPUShader(device, fragShader);
}

void addGlyphToVertices(
	TTF_GPUAtlasDrawSequence *sequence,
	std::vector<RenderVertex> *vertices,
	std::vector<Uint16> *indices,
	SDL_FColor color,
	glm::vec3 origin
) {
  for (int i=0; i < sequence->num_vertices; i++) {
		RenderVertex vert;
		const SDL_FPoint pos = sequence->xy[i];
		const SDL_FPoint uv = sequence->uv[i];
		vert.pos.x = origin.x + pos.x; vert.pos.y = pos.y - origin.y; vert.pos.z = origin.z;
		vert.uv.x = uv.x; vert.uv.y = uv.y;
		vertices->push_back(vert);
	}
	for (int i=0; i < sequence->num_indices; i++) {
		indices->push_back(sequence->indices[i]);
	}
}

void TextPipeline::render(
  SDL_GPUCommandBuffer *cmdBuf, SDL_GPURenderPass *pass,
	SDL_GPUTexture* target, glm::vec2 targetSize,
	std::vector<StringObject> &strings
) {
	if (strings.empty()) { return; }
	std::vector<RenderVertex> vertices;
	std::vector<Uint16> indices;
	SDL_GPUTexture *atlas = NULL;
	// process each StringObject individually
	for (int i=0; i<strings.size(); i++) {
		if (!strings[i].visible) continue;
		// move through sequence of glyphs
		strings[i].sequence = TTF_GetGPUTextDrawData(strings[i].ttfText);
		if (atlas == NULL) atlas = strings[i].sequence->atlas_texture;
		for (TTF_GPUAtlasDrawSequence *seq = strings[i].sequence; seq != NULL; seq = seq->next) {
			addGlyphToVertices(seq, &vertices, &indices, strings[i].color, strings[i].origin);
		}
	}
	if (vertices.size() < 1) { return; }
	copyVertexDataIntoBuffer(device, vertBuf, indexBuf, &vertices, &indices);

	bool internalPass = pass == NULL;
	if (internalPass) {
		pass = SDL_BeginGPURenderPass(cmdBuf, new SDL_GPUColorTargetInfo {
			.texture = target,
			.clear_color = SDL_FColor{ 0.02f, 0.02f, 0.08f, 1.0f },
			.load_op = SDL_GPU_LOADOP_LOAD,
			.store_op = SDL_GPU_STOREOP_STORE,
		}, 1, NULL);
	}

	// draw pipeline
  SDL_BindGPUGraphicsPipeline(pass, pipeline);
	SDL_BindGPUFragmentSamplers(pass, 0, new SDL_GPUTextureSamplerBinding {
    .texture = atlas,
    .sampler = sampler
  }, 1);
	SDL_BindGPUVertexBuffers(pass, 0, new SDL_GPUBufferBinding {
		.buffer = vertBuf,
		.offset = 0,
	}, 1);
	SDL_BindGPUIndexBuffer(pass, new SDL_GPUBufferBinding {
		.buffer = indexBuf,
		.offset = 0,
	}, SDL_GPU_INDEXELEMENTSIZE_16BIT);
	SDL_PushGPUVertexUniformData(cmdBuf, 0, &targetSize, sizeof(glm::vec2));
	int index_offset = 0, vertex_offset = 0;
	// dynamically offset buffers for each glyph
	for (int i=0; i<strings.size(); i++) {
		if (!strings[i].visible) continue;
		SDL_PushGPUFragmentUniformData(cmdBuf, 0, &strings[i].color, sizeof(SDL_FColor));
		for (TTF_GPUAtlasDrawSequence *seq = strings[i].sequence; seq != NULL; seq = seq->next) {
			SDL_DrawGPUIndexedPrimitives(pass, seq->num_indices, 1, index_offset, vertex_offset, 0);
			index_offset += seq->num_indices;
			vertex_offset += seq->num_vertices;
		}
	}

	if (internalPass) {
		SDL_EndGPURenderPass(pass);
	}
}

void TextPipeline::destroy() {
  SDL_ReleaseGPUBuffer(device, vertBuf);
  SDL_ReleaseGPUBuffer(device, indexBuf);
  SDL_ReleaseGPUSampler(device, sampler);
  SDL_ReleaseGPUGraphicsPipeline(device, pipeline);
}