#pragma once

#include <string>
#include <SDL3/SDL.h>
#include "textEngine.hpp"
#include "util.hpp"

// generic render pipeline
namespace App {
  class OverlayPipeline {
  public:
    OverlayPipeline(SDL_GPUTextureFormat targetFormat, SDL_GPUDevice *gpu, TextEngine *textEngine);
    SDL_GPUDevice *device = NULL;
    TextEngine *textEngine = NULL;
    SDL_GPUGraphicsPipeline *pipeline = NULL;
    SDL_GPUTexture *tx = NULL;
    void render(
      SDL_GPUCommandBuffer *cmdBuf, SDL_GPURenderPass *pass,
      SDL_GPUTexture* target, Vec2 screenSize
    );
    void destroy();
  };
}