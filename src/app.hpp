#pragma once

#include <vector>
#include <SDL3/SDL.h>

namespace App {
  class RenderInstance {
  public:
    RenderInstance(SDL_Window* window, SDL_GPUDevice *gpu);
    SDL_Window *win;
    SDL_GPUDevice *device;
    SDL_GPUShader *vertShader = NULL;
    SDL_GPUShader *fragShader = NULL;
    SDL_GPUGraphicsPipeline *pipeline = NULL;
    SDL_GPUCommandBuffer *cmdBuf = NULL;
    void renderToTexture();
    void renderToScreen();
    void destroy();
  };

  class AppState {
  public:
    SDL_Window *window = NULL;
    SDL_GPUDevice *gpu = NULL;
    RenderInstance *renderer = NULL;
  };
}