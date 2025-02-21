#include "sdfPipeline.hpp"

using namespace App;

SDFObject SDFObject::circle(glm::vec2 center, float radius) {
	SDFObject obj;
	obj.type = SDF_Circle;
	obj.center = center;
	obj.radius = radius;
	return obj;
}

SDFObject SDFObject::line(glm::vec2 p1, glm::vec2 p2, float thickness) {
	SDFObject obj;
	obj.type = SDF_Line;
	obj.center = p1;
	obj.v2 = p2;
	obj.thickness = thickness;
	return obj;
}

SDFObject SDFObject::triangle(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3) {
	SDFObject obj;
	obj.type = SDF_Triangle;
	obj.center = p1;
	obj.v2 = p2;
	obj.v3 = p3;
	return obj;
}

SDFObject SDFObject::rect(glm::vec2 center, glm::vec2 size) {
	SDFObject obj;
	obj.type = SDF_Rect;
	obj.center = center;
	obj.v2 = size;
	return obj;
}

SDFObject SDFObject::rect(glm::vec2 center, glm::vec2 size, float rotateDeg) {
	SDFObject obj;
	obj.type = SDF_RectA;
	obj.center = center;
	obj.v2 = size;
	obj.rotation = rotateDeg;
	return obj;
}

void SDFObject::withColor(SDL_FColor color) {
	this->color = color;
}

void SDFObject::withRoundCorner(float radius) {
	this->cornerRadius = radius;
}

void SDFObject::asOutline(float thickness) {
	this->thickness = thickness;
}

void SDFObject::updatePositionDelta(glm::vec2 delta) {
	if (type == SDF_Line || type == SDF_Triangle) {
		v2 = v2 + delta;
		v3 = v3 + delta;
	}
	center = center + delta;
}

void SDFObject::updatePosition(glm::vec2 newCenter) {
	glm::vec2 delta = newCenter - center;
	updatePositionDelta(delta);
}

SDFRenderObject SDFObject::renderObject() {
	Uint32 objType = 0;
	switch (type) {
		case SDF_Circle:
			objType = 1;
			break;
		case SDF_Line:
			objType = 2;
			break;
		case SDF_Triangle:
			objType = 3;
			break;
		case SDF_Rect:
			objType = 4;
			break;
		case SDF_RectA:
			objType = 5;
			break;
		default:
			break;
	}

	return SDFRenderObject {
		.objType = objType,
		.radius = radius,
		.center = center,
		.v2 = v2,
		.v3 = v3,
		.cornerRadius = cornerRadius,
		.rotation = rotation,
		.thickness = thickness,
		.color = color,
	};
}

#pragma region SDFPipeline

SDFPipeline::SDFPipeline(SDL_GPUTextureFormat targetFormat, SDL_GPUDevice *gpu) {
  device = gpu;
  // create shaders
  SDL_GPUShader *vertShader = App::loadShader(device, "fullScreenQuad.vert", 0, 0, 0, 0);
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
	// create storage buffer for objects
	objsBuffer = SDL_CreateGPUBuffer(device, new SDL_GPUBufferCreateInfo {
		.usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ,
		.size = 1000 * sizeof(SDFRenderObject),
	});
	// release shaders
	SDL_ReleaseGPUShader(device, vertShader);
  SDL_ReleaseGPUShader(device, fragShader);
}

void SDFPipeline::refreshObjects(std::vector<SDFObject> &objs) {
	Uint32 objsSize = sizeof(SDFRenderObject) * objs.size();
	// update object buffer with new data
	SDL_GPUTransferBuffer *transferBuf = SDL_CreateGPUTransferBuffer(
		device,
		new SDL_GPUTransferBufferCreateInfo {
			.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
			.size = objsSize,
		}
	);
	SDFRenderObject* objData = static_cast<SDFRenderObject*>(SDL_MapGPUTransferBuffer(
		device, transferBuf, false
	));
	for (int i=0; i < objs.size(); i++) {
		objData[i] = objs.at(i).renderObject();
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

void SDFPipeline::render(
	SDL_GPUCommandBuffer *cmdBuf, SDL_GPURenderPass *pass,
	SDL_GPUTexture* target, SDFSysData sys
) {
	SDL_BindGPUGraphicsPipeline(pass, pipeline);
	SDL_PushGPUFragmentUniformData(cmdBuf, 0, &sys, sizeof(SDFSysData));
	SDL_BindGPUFragmentStorageBuffers(pass, 0, &objsBuffer, 1);
	SDL_DrawGPUPrimitives(pass, 6, 1, 0, 0);
}

void SDFPipeline::destroy() {
	SDL_ReleaseGPUBuffer(device, objsBuffer);
  SDL_ReleaseGPUGraphicsPipeline(device, pipeline);
}

#pragma endregion SDFRenderer

#pragma region SDF math

float App::sdfToCir(glm::vec2 point, glm::vec2 center, float radius) {
	glm::vec2 v = center - point;
  return glm::length(v) - radius;
};

float App::sdfToLine(glm::vec2 point, glm::vec2 p1, glm::vec2 p2) {
	glm::vec2 pa = point - p1;
	glm::vec2 ba = p2 - p1;
	float h = SDL_clamp(glm::dot(pa, ba) / glm::dot(ba, ba), 0.0f, 1.0f);
	return glm::length(pa - ba * h);
}

float App::sdfToTriangle(glm::vec2 point, glm::vec2 p0, glm::vec2 p1, glm::vec2 p2) {
	glm::vec2 e0 = p1 - p0;
	glm::vec2 v0 = point - p0;
	glm::vec2 d0 = v0 - e0 * SDL_clamp(dot(v0, e0) / dot(e0, e0), 0.0f, 1.0f);
	float d0d = glm::dot(d0, d0);
	glm::vec2 e1 = p2 - p1;
	glm::vec2 v1 = point - p1;
	glm::vec2 d1 = v1 - e1 * SDL_clamp(dot(v1, e1) / dot(e1, e1), 0.0f, 1.0f);
	float d1d = glm::dot(d1, d1);
	glm::vec2 e2 = p0 - p2;
	glm::vec2 v2 = point - p2;
	glm::vec2 d2 = v2 - e2 * SDL_clamp(dot(v2, e2) / dot(e2, e2), 0.0f, 1.0f);
	float d2d = glm::dot(d2, d2);

	float o = e0.x * e2.y - e0.y * e2.x;
	float y0 = o * (v0.x * e0.y - v0.y * e0.x);
	float y1 = o * (v1.x * e1.y - v1.y * e1.x);
	float y2 = o * (v2.x * e2.y - v2.y * e2.x);
	float minD = d0d;
	if (d1d < minD) minD = d1d;
	if (d2d < minD) minD = d2d;
	float minY = y0;
	if (y1 < minY) minY = y1;
	if (y2 < minY) minY = y2;
	float sign = minY > 0.0f ? -1.0f : 1.0f;

	return SDL_sqrtf(minD) * sign;
}

float App::sdfToRect(glm::vec2 point, glm::vec2 center, glm::vec2 size) {
	glm::vec2 absP = point - center;
	if (absP.x < 0.0f) absP.x = -1.0f * absP.x;
	if (absP.y < 0.0f) absP.y = -1.0f * absP.y;
	glm::vec2 d0 = absP - size;
	glm::vec2 d = d0;
	if (d.x < 0.0f) d.x = 0.0f;
	if (d.y < 0.0f) d.y = 0.0f;
	float outer = glm::length(d);
	float inner = SDL_min(SDL_max(d0.x, d0.y), 0.0f);
	return outer + inner;
}

float App::sdfWithCorner(float sdf, float radius) {
	return sdf - radius;
}

float App::sdfAsOutline(float sdf, float thickness) {
	return SDL_fabsf(sdf) - thickness;
}

float App::calculateSdf(glm::vec2 point, float maxDist, std::vector<SDFObject> *objs) {
	float sdf = maxDist;
	for (int i=0; i < objs->size(); i++) {
		SDFRenderObject obj = objs->at(i).renderObject();
		float d = maxDist;
		switch (obj.objType) {
			case 1:
				d = sdfToCir(point, obj.center, obj.radius);
				break;
			case 2:
				d = sdfToLine(point, obj.center, obj.v2);
				break;
			case 3:
				d = sdfToTriangle(point, obj.center, obj.v2, obj.v3);
				break;
			case 4:
				d = sdfToRect(point, obj.center, obj.v2);
				break;
			default:
				break;
		}
		if (obj.cornerRadius > 0.0f) d = sdfWithCorner(d, obj.cornerRadius);
		if (obj.thickness > 0.0f) d = sdfAsOutline(d, obj.thickness);
		if (d < sdf) sdf = d;
	}
	return sdf;
}

float App::calculateRayMarch(glm::vec2 point, glm::vec2 target, float maxDist, std::vector<SDFObject> *objs) {
	glm::vec2 dir = glm::normalize(target - point);
	glm::vec2 p = point;
	float sdf = calculateSdf(p, maxDist, objs);
  float rayDist = sdf;
	for (int i=0; i < 1000; i++) {
    p = p + dir * sdf;
    sdf = calculateSdf(p, maxDist, objs);
    rayDist += sdf;
    if (rayDist > maxDist || sdf < 0.01f) break;
  }
  if (rayDist > maxDist) rayDist = maxDist;
  return rayDist;
}

#pragma endregion SDF math
