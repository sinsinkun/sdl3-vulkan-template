#pragma once

#include <string>
#include <vector>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "util.hpp"

// generic render pipeline
namespace App {
  class OverlayPipeline {
  public:
    static const int MAX_VERT_COUNT = 2000;
    static const int MAX_INDEX_COUNT = 4000;
    OverlayPipeline(
      SDL_GPUTextureFormat targetFormat, SDL_GPUDevice *gpu,
      TTF_TextEngine* textEngine
    );
    SDL_GPUDevice *device = NULL;
    SDL_GPUGraphicsPipeline *pipeline = NULL;
    SDL_GPUSampler *sampler = NULL;
    // glyphs resources
    SDL_GPUBuffer *vertBuf = NULL;
    SDL_GPUBuffer *indexBuf = NULL;
    int vertCount = 0;
    int indexCount = 0;
    // font resources
    TTF_Font *font = NULL;
    TTF_Text *ttfText = NULL;
    void updateText(std::string text);
    void render(
      SDL_GPUCommandBuffer *cmdBuf, SDL_GPURenderPass *pass,
      SDL_GPUTexture* target, Vec2 screenSize
    );
    void destroy();
  private:
    void addGlyphToVertices(
      TTF_GPUAtlasDrawSequence *sequence, std::vector<RenderVertex> *vertices, 
      std::vector<Uint16> *indices, SDL_FColor color
    );
  };
}