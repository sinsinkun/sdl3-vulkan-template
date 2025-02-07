#include "overlayPipeline.hpp"

using namespace App;

OverlayPipeline::OverlayPipeline(
  SDL_GPUTextureFormat targetFormat, SDL_GPUDevice *gpu, TTF_TextEngine *textEngine
) {
  device = gpu;
  // create shaders
  SDL_GPUShader *vertShader = App::loadShader(device, "textOverlay.vert", 0, 1, 0, 0);
  SDL_GPUShader *fragShader = App::loadShader(device, "textOverlay.frag", 1, 0, 0, 0);
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

  // create font
  font = TTF_OpenFont("assets/NotoSerifCHB.ttf", 48);
	ttfText = TTF_CreateText(textEngine, font, "", 0);

  // release shaders
	SDL_ReleaseGPUShader(device, vertShader);
  SDL_ReleaseGPUShader(device, fragShader);
}

void OverlayPipeline::updateText(std::string text) {
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

void OverlayPipeline::addGlyphToVertices(
	TTF_GPUAtlasDrawSequence *sequence,
	std::vector<RenderVertex> *vertices,
	std::vector<Uint16> *indices,
	SDL_FColor color
) {
  for (int i=0; i < sequence->num_vertices; i++) {
		RenderVertex vert;
		const SDL_FPoint pos = sequence->xy[i];
		const SDL_FPoint uv = sequence->uv[i];
		vert.x = pos.x; vert.y = pos.y;
		vert.u = uv.x; vert.v = uv.y;
		vert.r = color.r; vert.g = color.g; vert.b = color.b; vert.a = color.a;
		vertices->push_back(vert);
	}
	for (int i=0; i < sequence->num_indices; i++) {
		indices->push_back(sequence->indices[i]);
	}
}

void OverlayPipeline::uploadVertices(std::vector<RenderVertex> *verts, std::vector<Uint16> *indices) {
	Uint32 vSize = sizeof(RenderVertex) * MAX_VERT_COUNT;
	Uint32 iSize = sizeof(Uint16) * MAX_INDEX_COUNT;

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
	Uint16* indexData = static_cast<Uint16*>(SDL_MapGPUTransferBuffer(
		device, idxTransferBuf, false
	));
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

void OverlayPipeline::render(
  SDL_GPUCommandBuffer *cmdBuf, SDL_GPURenderPass *pass,
  SDL_GPUTexture* target, Vec2 screenSize
) {
  // move through sequence of glyphs
  TTF_GPUAtlasDrawSequence *sequence = TTF_GetGPUTextDrawData(ttfText);
	std::vector<RenderVertex> vertices;
	std::vector<Uint16> indices;
  for (TTF_GPUAtlasDrawSequence *seq = sequence; seq != NULL; seq = seq->next) {
		addGlyphToVertices(seq, &vertices, &indices, SDL_FColor{1.0f, 0.5f, 1.0f, 1.0f});
  }
	uploadVertices(&vertices, &indices);

	// draw pipeline
  SDL_BindGPUGraphicsPipeline(pass, pipeline);
	SDL_BindGPUVertexBuffers(pass, 0, new SDL_GPUBufferBinding {
		.buffer = vertBuf,
		.offset = 0,
	}, 1);
	SDL_BindGPUIndexBuffer(pass, new SDL_GPUBufferBinding {
		.buffer = indexBuf,
		.offset = 0,
	}, SDL_GPU_INDEXELEMENTSIZE_16BIT);
	SDL_PushGPUVertexUniformData(cmdBuf, 0, &screenSize, sizeof(Vec2));
  SDL_BindGPUFragmentSamplers(pass, 0, new SDL_GPUTextureSamplerBinding {
    .texture = sequence->atlas_texture,
    .sampler = sampler
  }, 1);
	int index_offset = 0, vertex_offset = 0;
	// dynamically offset buffers for each glyph
	for (TTF_GPUAtlasDrawSequence *seq = sequence; seq != NULL; seq = seq->next) {
		SDL_BindGPUFragmentSamplers(pass, 0, new SDL_GPUTextureSamplerBinding {
			.texture = seq->atlas_texture,
			.sampler = sampler
		}, 1);
		SDL_DrawGPUIndexedPrimitives(pass, seq->num_indices, 1, index_offset, vertex_offset, 0);
		index_offset += seq->num_indices;
		vertex_offset += seq->num_vertices;
	}
}

void OverlayPipeline::destroy() {
  TTF_CloseFont(font);
  SDL_ReleaseGPUBuffer(device, vertBuf);
  SDL_ReleaseGPUBuffer(device, indexBuf);
  SDL_ReleaseGPUSampler(device, sampler);
  SDL_ReleaseGPUGraphicsPipeline(device, pipeline);
}