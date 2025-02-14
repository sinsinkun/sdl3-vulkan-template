#pragma once

#include <string>
#include <vector>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
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
    glm::vec3 origin {0.0f, 0.0f, 0.0f};
    bool visible = true;
    void updateText(std::string text);
  };
  class TextPipeline {
  public:
    static const int MAX_VERT_COUNT = 2000;
    static const int MAX_INDEX_COUNT = 4000;
    TextPipeline(SDL_GPUTextureFormat targetFormat, SDL_GPUDevice *gpu);
    void render(
      SDL_GPUCommandBuffer *cmdBuf, SDL_GPURenderPass *pass,
      SDL_GPUTexture* target, glm::vec2 targetSize,
      std::vector<StringObject> strings
    );
    void destroy();
  private:
    SDL_GPUDevice *device = NULL;
    SDL_GPUGraphicsPipeline *pipeline = NULL;
    SDL_GPUSampler *sampler = NULL;
    // glyphs resources
    SDL_GPUBuffer *vertBuf = NULL;
    SDL_GPUBuffer *indexBuf = NULL;
    int vertCount = 0;
    int indexCount = 0;
  };
}