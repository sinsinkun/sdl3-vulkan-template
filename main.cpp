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

  state.window = SDL_CreateWindow("SDL3 Vulkan", 800, 600, SDL_WINDOW_RESIZABLE);
  if (!state.window) {
    SDL_Log("SDL_CreateWindow() failed: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  SDL_Log("Window initialized");
  SDL_SetWindowMinimumSize(state.window, 400, 300);

  // can add other shader formats: SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL
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

  state.renderer = new SDFRenderer(state.window, state.gpu);

  SDFObject cir1 = SDFObject::circle(Vec2 { 500.0f, 450.0f }, 38.0f);
  cir1.withColor(SDL_FColor {1.0f, 0.0f, 0.0f, 1.0f});
  SDFObject cir2 = SDFObject::circle(Vec2 { 300.0f, 200.0f }, 50.0f);
  cir2.withColor(SDL_FColor {0.0f, 0.5f, 0.5f, 1.0f});
  SDFObject rect1 = SDFObject::rect(Vec2 { 400.0f, 200.0f }, Vec2 { 50.0f, 60.0f });
  rect1.withColor(SDL_FColor {0.0f, 1.0f, 0.0f, 0.8f});
  rect1.withRoundCorner(10.0f);
  state.sdfObjects.push_back(cir1);
  state.sdfObjects.push_back(cir2);
  state.sdfObjects.push_back(rect1);
  state.renderer->refreshObjects(state.sdfObjects);

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
      if (event->key.scancode == SDL_SCANCODE_F1) {
        state.printFps = true;
      }
      break;
    case SDL_EVENT_KEY_UP:
      if (event->key.scancode == SDL_SCANCODE_F1) {
        state.printFps = false;
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

  // calculate FPS
  Uint64 newTime = SDL_GetTicks();
  Uint64 delta = newTime - state.lifetime;
  state.lifetime = SDL_GetTicks();
  if (state.printFps && state.timeSinceLastFps > 600) {
    state.timeSinceLastFps = 0;
    float fps = 1001.0f;
    if (delta != 0) fps = 1000.0f / delta;
    SDL_Log("FPS: %.2f", fps);
  } else {
    state.timeSinceLastFps += delta;
  }

  int err = state.renderer->renderToScreen(SDFSysData {
    .screenSize = Vec2(0.0f),
    .lightPos = Vec2(0.0f),
    .lightColor = SDL_FColor{0.2f, 0.2f, 0.5f, 0.8f},
    .lightDist = 0.0f,
  });
  if (err != 0) {
    SDL_Log("Render to screen failure");
    return SDL_APP_FAILURE;
  }
  return SDL_APP_CONTINUE;
}

// clean up on exit
void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  AppState& state = *static_cast<AppState*>(appstate);
  SDL_Log("Closing SDL3");
  state.renderer->destroy();
  delete state.renderer;
  SDL_ReleaseWindowFromGPUDevice(state.gpu, state.window);
  SDL_DestroyGPUDevice(state.gpu);
  SDL_DestroyWindow(state.window);
  SDL_Quit();
}