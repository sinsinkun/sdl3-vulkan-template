#pragma once

#include <vector>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <glm/vec2.hpp>

#include "util.hpp"
#include "sdfPipeline.hpp"
#include "textPipeline.hpp"

namespace App {
  struct AppState {
    SDL_Window *window = NULL;
    SDL_GPUDevice *gpu = NULL;
    SDL_Surface *winIcon = NULL;
    TTF_TextEngine *textEngine = NULL;
    TTF_Font *font = NULL;
    TextPipeline *overlayp = NULL;
    std::vector<StringObject> overlayStrs;
    glm::vec2 mousePosScreenSpace = glm::vec2(0.0f);
    glm::vec2 winSize = glm::vec2(800.0f, 600.0f);
    // SDF render
    Uint64 sdfTime = 0;
    glm::vec2 sdfLightPos = glm::vec2(0.0f);
    std::vector<SDFObject> sdfObjects;
    SDFPipeline *sdfp = NULL;
    SDFPipeline *sdfpDebug = NULL;
    bool sdfPosUpdate = true;
    // FPS debug helpers
    Uint64 lifetime = 0;
    Uint64 timeSinceLastFps = 0;
  };
}