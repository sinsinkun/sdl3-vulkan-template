#pragma once

#include <vector>
#include <SDL3/SDL.h>

#include "renderer.hpp"

namespace App {
  class AppState {
  public:
    SDL_Window *window = NULL;
    SDL_GPUDevice *gpu = NULL;
    RenderInstance *renderer = NULL;
  };
}