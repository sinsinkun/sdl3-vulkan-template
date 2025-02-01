#pragma once

#include <vector>
#include <SDL3/SDL.h>
#include "util.hpp"

namespace App {
  struct SDFRenderObject {
    Uint32 objType = 0;
    float radius = 0.0f;
    Vec2 center = Vec2 { 0.0f, 0.0f }; // 1st quad
    Vec2 v2 = Vec2 { 0.0f, 0.0f };
    Vec2 v3 = Vec2 { 0.0f, 0.0f }; // 2nd quad
    float cornerRadius = 0.0f;
    float rotation = 0.0f;
    float thickness = 0.0f;
    float padding = 0.0f; // 3rd quad
    Vec4 color = Vec4{ 1.0f, 1.0f, 1.0f, 1.0f }; // 4th quad
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
    void withColor(Vec4 color);
    void withRoundCorner(float radius);
    void asOutline(float thickness);
    SDFRenderObject renderObject();
  protected:
    SDFObjectType type = SDF_None;
    Vec2 center = Vec2 { 0.0f, 0.0f };
    float radius = 0.0f;
    float cornerRadius = 0.0f;
    float rotation = 0.0f;
    float thickness = 0.0f;
    Vec2 v2 = Vec2 { 0.0f, 0.0f };
    Vec2 v3 = Vec2 { 0.0f, 0.0f };
    Vec4 color = Vec4{ 1.0f, 1.0f, 1.0f, 1.0f };
  };
  class SDFRenderer {
  public:
    SDFRenderer(SDL_Window* window, SDL_GPUDevice *gpu);
    SDL_Window *win;
    SDL_GPUDevice *device;
    SDL_GPUGraphicsPipeline *pipeline = NULL;
    SDL_GPUBuffer *objsBuffer = NULL;
    void refreshObjects(std::vector<SDFObject> objs);
    int renderToScreen();
    void destroy();
  };
}