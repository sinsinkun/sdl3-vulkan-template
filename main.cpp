#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "src/app.hpp"

using namespace App;

// initialization of app
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
  SDL_SetAppMetadata("SDL-Test", "1.0", "com.example.sdl-test");

  *appstate = new AppState;
  AppState& state = *static_cast<AppState*>(*appstate);

  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
    SDL_Log("SDL_Init(SDL_INIT_VIDEO) failed: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  SDL_Log(
    "SDL3 v%d.%d.%d initialized\n",
    SDL_VERSIONNUM_MAJOR(SDL_VERSION),
    SDL_VERSIONNUM_MINOR(SDL_VERSION),
    SDL_VERSIONNUM_MICRO(SDL_VERSION)
  );

  state.window = SDL_CreateWindow("SDL3 Vulkan", 800, 600, SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);
  if (!state.window) {
    SDL_Log("SDL_CreateWindow() failed: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  SDL_Log("Window initialized");
  SDL_SetWindowMinimumSize(state.window, 400, 300);

  state.gpu = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, NULL);
  if (!state.gpu) {
    SDL_Log("SDL_CreateGPUDevice() failed: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  SDL_Log("GPU device initialized: %s", SDL_GetGPUDeviceDriver(state.gpu));

  bool claimed = SDL_ClaimWindowForGPUDevice(state.gpu, state.window);
  if (!claimed) {
    SDL_Log("SDL_ClaimWindowForGPUDevice() failed: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  SDL_Log("Claimed window for GPU device");

  state.renderer = new RenderInstance(state.window, state.gpu);

  return SDL_APP_CONTINUE;
}

// handle events
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  AppState& state = *static_cast<AppState*>(appstate);
  switch (event->type) {
    // triggers on last window close and other things. End the program.
    case SDL_EVENT_QUIT:  
      return SDL_APP_SUCCESS;
    case SDL_EVENT_KEY_DOWN:
      // quit if user hits ESC key
      if (event->key.scancode == SDL_SCANCODE_ESCAPE) {
        return SDL_APP_SUCCESS;
      }
      break;
    case SDL_EVENT_TEXT_INPUT:
      break;
    default:
      break;
  }
  return SDL_APP_CONTINUE;
}

// update/render loop
SDL_AppResult SDL_AppIterate(void *appstate) {
  AppState& state = *static_cast<AppState*>(appstate);
  
  int err = state.renderer->renderToScreen();
  if (err != 0) {
    SDL_Log("Render to screen failure");
    return SDL_APP_FAILURE;
  }
  return SDL_APP_CONTINUE;
}

// clean up on exit
void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  AppState& state = *static_cast<AppState*>(appstate);
  state.renderer->destroy();
  delete state.renderer;
  SDL_ReleaseWindowFromGPUDevice(state.gpu, state.window);
  SDL_DestroyGPUDevice(state.gpu);
  SDL_DestroyWindow(state.window);
  SDL_Quit();
}