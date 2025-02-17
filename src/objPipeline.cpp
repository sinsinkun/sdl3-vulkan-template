#include "objPipeline.hpp"

using namespace App;

ObjectPipeline::ObjectPipeline(SDL_GPUTextureFormat targetFormat, SDL_GPUDevice *gpu) {
  device = gpu;
  // create shaders
  SDL_GPUShader *vertShader = App::loadShader(device, "obj.vert", 0, 1, 0, 0);
  SDL_GPUShader *fragShader = App::loadShader(device, "obj.frag", 1, 1, 0, 0);

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

  // release shaders
	SDL_ReleaseGPUShader(device, vertShader);
  SDL_ReleaseGPUShader(device, fragShader);
}

int ObjectPipeline::uploadObject(std::vector<RenderVertex> &vertices) {
	// create vertex buffer
  Uint32 vSize = sizeof(RenderVertex) * vertices.size();
  SDL_GPUBuffer *vBuffer = SDL_CreateGPUBuffer(device, new SDL_GPUBufferCreateInfo {
    .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
    .size = vSize
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
  for (int i=0; i < vertices.size(); i++) {
    vertData[i] = vertices.at(i);
  }
  SDL_UnmapGPUTransferBuffer(device, vertTransferBuf);

  // create cmd buffer + copy pass
	SDL_GPUCommandBuffer *cmdBuf = SDL_AcquireGPUCommandBuffer(device);
	SDL_GPUCopyPass *copyPass = SDL_BeginGPUCopyPass(cmdBuf);
  // upload vertex buffer
  SDL_UploadToGPUBuffer(
    copyPass,
    new SDL_GPUTransferBufferLocation {
      .transfer_buffer = vertTransferBuf,
      .offset = 0,
    },
    new SDL_GPUBufferRegion {
      .buffer = vBuffer,
      .offset = 0,
      .size = vSize,
    },
    false
  );
  // clean up passes
	SDL_EndGPUCopyPass(copyPass);
	if (!SDL_SubmitGPUCommandBuffer(cmdBuf)) {
    SDL_Log("Failed to upload to buffer - %s", SDL_GetError());
  };
  // release transfer buffers
  SDL_ReleaseGPUTransferBuffer(device, vertTransferBuf);

  // create placeholder texture + sampler
  SDL_GPUTexture *tx = SDL_CreateGPUTexture(device, new SDL_GPUTextureCreateInfo {
    .type = SDL_GPU_TEXTURETYPE_2D,
    .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
    .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER,
    .width = 1,
    .height = 1,
    .layer_count_or_depth = 1,
    .num_levels = 1,
  });
  SDL_GPUSampler *sm = SDL_CreateGPUSampler(device, new SDL_GPUSamplerCreateInfo {
    .min_filter = SDL_GPU_FILTER_LINEAR,
    .mag_filter = SDL_GPU_FILTER_LINEAR,
    .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
    .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
    .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
    .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE
  });

  // create object
  int id = robjs.size();
  robjs.push_back(RenderObject {
    .id = id,
    .visible = true,
    .vertexBuffer = vBuffer,
    .vertexCount = (int)(vertices.size()),
    .indexCount = 0,
    .sampler = sm,
    .texture = tx,
  });
  return id;
}

int ObjectPipeline::uploadObject(std::vector<RenderVertex> &vertices, std::vector<Uint16> &indices) {
  // create vertex buffer
  Uint32 vSize = sizeof(RenderVertex) * vertices.size();
  SDL_GPUBuffer *vBuffer = SDL_CreateGPUBuffer(device, new SDL_GPUBufferCreateInfo {
    .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
    .size = vSize
  });
  // create index buffer
  Uint32 iSize = sizeof(Uint16) * indices.size();
  SDL_GPUBuffer *iBuffer = SDL_CreateGPUBuffer(device, new SDL_GPUBufferCreateInfo {
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
  for (int i=0; i < vertices.size(); i++) {
    vertData[i] = vertices.at(i);
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

  // upload vertex buffer
  SDL_UploadToGPUBuffer(
    copyPass,
    new SDL_GPUTransferBufferLocation {
      .transfer_buffer = vertTransferBuf,
      .offset = 0,
    },
    new SDL_GPUBufferRegion {
      .buffer = vBuffer,
      .offset = 0,
      .size = vSize,
    },
    false
  );
  // upload index buffer
  SDL_UploadToGPUBuffer(
    copyPass,
    new SDL_GPUTransferBufferLocation {
      .transfer_buffer = idxTransferBuf,
      .offset = 0,
    },
    new SDL_GPUBufferRegion {
      .buffer = iBuffer,
      .offset = 0,
      .size = iSize,
    },
    false
  );

  // clean up passes
	SDL_EndGPUCopyPass(copyPass);
	if (!SDL_SubmitGPUCommandBuffer(cmdBuf)) {
    SDL_Log("Failed to upload to buffers - %s", SDL_GetError());
  };
  // release transfer buffers
  SDL_ReleaseGPUTransferBuffer(device, vertTransferBuf);
  SDL_ReleaseGPUTransferBuffer(device, idxTransferBuf);

  
  // create placeholder texture + sampler
  SDL_GPUTexture *tx = SDL_CreateGPUTexture(device, new SDL_GPUTextureCreateInfo {
    .type = SDL_GPU_TEXTURETYPE_2D,
    .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
    .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER,
    .width = 1,
    .height = 1,
    .layer_count_or_depth = 1,
    .num_levels = 1,
  });
  SDL_GPUSampler *sm = SDL_CreateGPUSampler(device, new SDL_GPUSamplerCreateInfo {
    .min_filter = SDL_GPU_FILTER_LINEAR,
    .mag_filter = SDL_GPU_FILTER_LINEAR,
    .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
    .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
    .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
    .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE
  });

  // create object
  int id = robjs.size();
  robjs.push_back(RenderObject {
    .id = id,
    .visible = true,
    .vertexBuffer = vBuffer,
    .indexBuffer = iBuffer,
    .vertexCount = (int)(vertices.size()),
    .indexCount = (int)(indices.size()),
    .sampler = sm,
    .texture = tx,
  });
  return id;
}

void ObjectPipeline::addTextureToObject(int id, SDL_GPUTextureFormat txFormat, Uint32 w, Uint32 h, bool isRenderTarget) {
  if (id >= robjs.size()) {
    SDL_Log("ERR: Tried to access render object that doesn't exist %d", id);
    return;
  }
  SDL_ReleaseGPUTexture(device, robjs.at(id).texture);
  SDL_GPUTextureUsageFlags usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;
  if (isRenderTarget) {
    usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;
  }
  // todo: handle uploading static image
  robjs.at(id).texture = SDL_CreateGPUTexture(device, new SDL_GPUTextureCreateInfo {
    .type = SDL_GPU_TEXTURETYPE_2D,
    .format = txFormat,
    .usage = usage,
    .width = w,
    .height = h,
    .layer_count_or_depth = 1,
    .num_levels = 1,
  });
}

void ObjectPipeline::updateObjectPos(int id, glm::vec3 pos) {
  if (id >= robjs.size()) {
    SDL_Log("ERR: Tried to access render object that doesn't exist %d", id);
    return;
  }
  robjs.at(id).pos = pos;
}

void ObjectPipeline::updateObjectRot(int id, glm::vec3 rotAxis, float rotAngle) {
  if (id >= robjs.size()) {
    SDL_Log("ERR: Tried to access render object that doesn't exist %d", id);
    return;
  }
  robjs.at(id).rotAxis = rotAxis;
  robjs.at(id).rotAngleRad = rotAngle;
}

void ObjectPipeline::updateObjectScale(int id, glm::vec3 scale) {
  if (id >= robjs.size()) {
    SDL_Log("ERR: Tried to access render object that doesn't exist %d", id);
    return;
  }
  robjs.at(id).scale = scale;
}

void ObjectPipeline::updateCamera(RenderCamera cam) {
  this->cam = cam;
}

glm::mat4x4 modelMatrix(RenderObject const &obj) {
  glm::mat4x4 id = glm::mat4(1.0f);
  glm::mat4x4 model = glm::translate(id, obj.pos);
  model = glm::rotate(model, obj.rotAngleRad, obj.rotAxis);
  model = glm::scale(model, obj.scale);
  return model;
}

glm::mat4x4 viewMatrix(RenderCamera const &cam) {
  return glm::lookAt(cam.pos, cam.lookAt, cam.up);
}

glm::mat4x4 projMatrix(RenderCamera const &cam) {
  if (cam.perspective) {
    return glm::perspective(cam.fovY, cam.viewWidth / cam.viewHeight, cam.near, cam.far);
  } else {
    float hw = cam.viewWidth / 2.0f;
    float hh = cam.viewHeight / 2.0f;
    return glm::ortho(-hw, hw, -hh, hh);
  }
}

void ObjectPipeline::render(
  SDL_GPUCommandBuffer *cmdBuf, SDL_GPURenderPass *pass,
  SDL_GPUTexture* target, glm::vec2 targetSize
) {
  SDL_BindGPUGraphicsPipeline(pass, pipeline);
  // build view/proj matrices early
  glm::mat4x4 view = viewMatrix(cam);
  glm::mat4x4 proj = projMatrix(cam);

  // handle each object separately
  for (RenderObject const &obj : robjs) {
    if (!obj.visible) continue;
    if (obj.vertexBuffer == NULL) {
      SDL_Log("ERR: Missing vertex data for object %d", obj.id);
      continue;
    }
    SDL_BindGPUVertexBuffers(pass, 0, new SDL_GPUBufferBinding {
      .buffer = obj.vertexBuffer,
      .offset = 0,
    }, 1);
    // build matrices
    glm::mat4x4 matrices[3] = { modelMatrix(obj), view, proj };
    SDL_PushGPUVertexUniformData(cmdBuf, 0, &matrices, sizeof(matrices));
    // upload texture
    SDL_BindGPUFragmentSamplers(pass, 0, new SDL_GPUTextureSamplerBinding {
      .texture = obj.texture,
      .sampler = obj.sampler
    }, 1);
    // draw
    if (obj.indexCount > 0) {
      SDL_BindGPUIndexBuffer(pass, new SDL_GPUBufferBinding {
        .buffer = obj.indexBuffer,
        .offset = 0,
      }, SDL_GPU_INDEXELEMENTSIZE_16BIT);
      SDL_DrawGPUIndexedPrimitives(pass, obj.indexCount, 1, 0, 0, 0);
    } else {
      SDL_DrawGPUPrimitives(pass, obj.vertexCount, 1, 0, 0);
    }
  }
}

void ObjectPipeline::destroy() {
  for (int i=0; i<robjs.size(); i++) {
    if (robjs[i].vertexBuffer != NULL) SDL_ReleaseGPUBuffer(device, robjs[i].vertexBuffer);
    if (robjs[i].indexBuffer != NULL) SDL_ReleaseGPUBuffer(device, robjs[i].indexBuffer);
    if (robjs[i].texture != NULL) SDL_ReleaseGPUTexture(device, robjs[i].texture);
    if (robjs[i].sampler != NULL) SDL_ReleaseGPUSampler(device, robjs[i].sampler);
  }
  robjs.clear();
  SDL_ReleaseGPUBuffer(device, debugVertBuffer);
  SDL_ReleaseGPUGraphicsPipeline(device, pipeline);
}