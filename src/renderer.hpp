#pragma once

#include <vector>
#include <SDL3/SDL.h>

namespace App {
  enum GPUPrimitiveType { Point, Line, Triangle };
  struct RenderVertex {
    float x, y, z;
    float u, v;
    float nx, ny, nz;
  };
  class RenderObject {
  public:
    SDL_GPUBuffer *vertexBuffer = NULL;
    SDL_GPUBuffer *indexBuffer = NULL;
    int vertexCount = 0;
    int indexCount = 0;
  };
  class RenderInstance {
  public:
    RenderInstance(SDL_Window* window, SDL_GPUDevice *gpu);
    SDL_Window *win;
    SDL_GPUDevice *device;
    SDL_GPUGraphicsPipeline *pipeline = NULL;
    // int addRenderObject();
    // void updateRenderObject();
    // int renderToTexture();
    int renderToScreen();
    void destroy();
  private:
    std::vector<RenderObject> renderObjects;
    void uploadVertices(std::vector<RenderVertex> verts);
    void uploadVertices(std::vector<RenderVertex> verts, std::vector<Uint16> indices);
  };
  // SDF render definitions
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
    Vec4 color = Vec4{ 0.0f, 0.0f, 0.0f, 1.0f }; // 4th quad
  };
  class SDFRenderer {
  public:
    SDFRenderer(SDL_Window* window, SDL_GPUDevice *gpu);
    SDL_Window *win;
    SDL_GPUDevice *device;
    SDL_GPUGraphicsPipeline *pipeline = NULL;
    SDL_GPUBuffer *objsBuffer = NULL;
    void updateObjects();
    int renderToScreen();
    void destroy();
  };
}