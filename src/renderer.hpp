#pragma once

#include <vector>
#include <SDL3/SDL.h>
#include "util.hpp"

// generic render pipeline
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
}