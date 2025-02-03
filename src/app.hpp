#pragma once

#include <vector>
#include <SDL3/SDL.h>

#include "util.hpp"
#include "renderer.hpp"
#include "sdfRenderer.hpp"
#include "textEngine.hpp"

namespace App {
  struct AppState {
    SDL_Window *window = NULL;
    SDL_GPUDevice *gpu = NULL;
    TextEngine textEngine;
    Vec2 mousePosScreenSpace = Vec2(0.0f);
    // SDF render
    std::vector<SDFObject> sdfObjects;
    SDFPipeline *sdfp = NULL;
    // FPS debug helpers
    Uint64 lifetime = 0;
    Uint64 timeSinceLastFps = 0;
    bool printFps = false;
  };
}