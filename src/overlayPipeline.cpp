#include "overlayPipeline.hpp"

using namespace App;

OverlayPipeline::OverlayPipeline(
  SDL_GPUTextureFormat targetFormat, SDL_GPUDevice *gpu, TextEngine *textEngine
) {
  device = gpu;
  // create shaders
  SDL_GPUShader *vertShader = App::loadShader(device, "fullScreenQuad.vert", 0, 0, 0, 0);
  SDL_GPUShader *fragShader = App::loadShader(device, "textOverlay.frag", 0, 1, 0, 1);
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
  // create texture
  tx = SDL_CreateGPUTexture(device, new SDL_GPUTextureCreateInfo {
    .type = SDL_GPU_TEXTURETYPE_2D,
    .format = SDL_GPU_TEXTUREFORMAT_R8_UINT,
    .usage = SDL_GPU_TEXTUREUSAGE_GRAPHICS_STORAGE_READ,
    .width = 800,
    .height = 600,
    .layer_count_or_depth = 1,
    .num_levels = 1,
  });
  SDL_SetGPUTextureName(device, tx, "Overlay Texture");

  // release shaders
	SDL_ReleaseGPUShader(device, vertShader);
  SDL_ReleaseGPUShader(device, fragShader);
}

void OverlayPipeline::render(
  SDL_GPUCommandBuffer *cmdBuf, SDL_GPURenderPass *pass,
  SDL_GPUTexture* target, glm::vec2 screenSize
) {
  SDL_BindGPUGraphicsPipeline(pass, pipeline);
  SDL_BindGPUFragmentStorageTextures(pass, 0, &tx, 1);
  SDL_PushGPUFragmentUniformData(cmdBuf, 0, &screenSize, sizeof(glm::vec2));
  SDL_DrawGPUPrimitives(pass, 6, 1, 0, 0);
}

void OverlayPipeline::destroy() {
  SDL_ReleaseGPUTexture(device, tx);
  SDL_ReleaseGPUGraphicsPipeline(device, pipeline);
}