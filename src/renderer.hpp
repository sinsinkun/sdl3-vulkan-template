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
  class RenderInstance {
  public:
    RenderInstance(SDL_Window* window, SDL_GPUDevice *gpu);
    SDL_Window *win;
    SDL_GPUDevice *device;
    SDL_GPUGraphicsPipeline *pipeline = NULL;
    SDL_GPUBuffer *vertexBuffer = NULL;
    SDL_GPUBuffer *indexBuffer = NULL;
    SDL_GPUTexture *srcTexture = NULL;
    SDL_GPUTexture *dstTexture = NULL;
    SDL_GPUSampler *sampler = NULL;
    // int renderToTexture();
    int renderToScreen();
    void destroy();
  private:
    void uploadVertices(std::vector<RenderVertex> verts);
    void uploadVertices(std::vector<RenderVertex> verts, std::vector<Uint16> indices);
  };
}