#pragma once

#include <vector>
#include <SDL3/SDL.h>
#include "util.hpp"

namespace App {
  struct SDFRenderObject {
    Uint32 objType = 0;
    float radius = 0.0f;
    Vec2 center = Vec2(0.0f); // 1st quad
    Vec2 v2 = Vec2(0.0f);
    Vec2 v3 = Vec2(0.0f); // 2nd quad
    float cornerRadius = 0.0f;
    float rotation = 0.0f;
    float thickness = 0.0f;
    float padding = 0.0f; // 3rd quad
    SDL_FColor color = WHITE; // 4th quad
  };
  enum SDFObjectType {
    SDF_None, SDF_Circle, SDF_Line, SDF_Triangle, SDF_Rect, SDF_RectA
  };
  class SDFObject {
  public:
    SDFObject() {};
    static SDFObject circle(Vec2 center, float radius);
    static SDFObject line(Vec2 p1, Vec2 p2, float thickness);
    static SDFObject triangle(Vec2 p1, Vec2 p2, Vec2 p3);
    static SDFObject rect(Vec2 center, Vec2 size);
    static SDFObject rect(Vec2 center, Vec2 size, float rotateDeg);
    void withColor(SDL_FColor color);
    void withRoundCorner(float radius);
    void asOutline(float thickness);
    void updatePositionDelta(Vec2 delta);
    void updatePosition(Vec2 center);
    SDFRenderObject renderObject();
  protected:
    SDFObjectType type = SDF_None;
    Vec2 center = Vec2(0.0f);
    float radius = 0.0f;
    float cornerRadius = 0.0f;
    float rotation = 0.0f;
    float thickness = 0.0f;
    Vec2 v2 = Vec2(0.0f);
    Vec2 v3 = Vec2(0.0f);
    SDL_FColor color = WHITE;
  };
  struct SDFSysData {
    Vec2 screenSize;
    Vec2 lightPos;
    SDL_FColor lightColor;
    float lightDist;
  };
  class SDFPipeline {
  public:
    SDFPipeline(SDL_GPUTextureFormat targetFormat, SDL_GPUDevice *gpu);
    SDL_GPUDevice *device;
    SDL_GPUGraphicsPipeline *pipeline = NULL;
    SDL_GPUBuffer *objsBuffer = NULL;
    void refreshObjects(std::vector<SDFObject> objs);
    void render(
      SDL_GPUCommandBuffer *cmdBuf, SDL_GPURenderPass *pass,
      SDL_GPUTexture* target, SDFSysData sys
    );
    void destroy();
  };
}