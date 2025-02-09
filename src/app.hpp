#pragma once

#include <vector>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "util.hpp"
#include "sdfPipeline.hpp"
#include "textPipeline.hpp"

namespace App {
  struct AppState {
    SDL_Window *window = NULL;
    SDL_GPUDevice *gpu = NULL;
    TTF_TextEngine *textEngine = NULL;
    TTF_Font *font = NULL;
    TextPipeline *overlayp = NULL;
    Vec2 mousePosScreenSpace = Vec2(0.0f);
    Vec2 winSize = Vec2(0.0f);
    // SDF render
    std::vector<SDFObject> sdfObjects;
    SDFPipeline *sdfp = NULL;
    SDFPipeline *sdfpDebug = NULL;
    bool sdfPosUpdate = true;
    // FPS debug helpers
    Uint64 lifetime = 0;
    Uint64 timeSinceLastFps = 0;
  };
}