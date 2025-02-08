#pragma once

#include <string>
#include <vector>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "util.hpp"

// generic render pipeline
namespace App {
  class StringObject {
  public:
    StringObject(TTF_TextEngine *textEngine, TTF_Font* font, std::string text);
    std::string text;
    TTF_Text *ttfText = NULL;
    TTF_GPUAtlasDrawSequence *sequence = NULL;
    SDL_FColor color = WHITE;
    Vec3 origin {0.0f, 0.0f, 0.0f };
    void updateText(std::string text);
  };
  class TextPipeline {
  public:
    static const int MAX_VERT_COUNT = 2000;
    static const int MAX_INDEX_COUNT = 4000;
    TextPipeline(SDL_GPUTextureFormat targetFormat, SDL_GPUDevice *gpu);
    SDL_GPUDevice *device = NULL;
    SDL_GPUGraphicsPipeline *pipeline = NULL;
    SDL_GPUSampler *sampler = NULL;
    // glyphs resources
    SDL_GPUBuffer *vertBuf = NULL;
    SDL_GPUBuffer *indexBuf = NULL;
    int vertCount = 0;
    int indexCount = 0;
    std::vector<StringObject> strings;
    void render(
      SDL_GPUCommandBuffer *cmdBuf, SDL_GPURenderPass *pass,
      SDL_GPUTexture* target, Vec2 targetSize
    );
    void destroy();
  };
}