#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <glm/ext.hpp>

#include "src/app.hpp"

using namespace App;

SDL_AppResult setupSDL(AppState& state) {
  SDL_SetAppMetadata("SDL-Test", "1.0", "com.example.sdl-test");

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

  // load icon
  state.winIcon = IMG_Load("assets/icon.png");
  SDL_SetWindowIcon(state.window, state.winIcon);

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

  // maintain a lower poll rate
  SDL_SetHint(SDL_HINT_MAIN_CALLBACK_RATE, "10000");

  return SDL_APP_CONTINUE;
}

// initialization of app
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
  *appstate = new AppState;
  AppState& state = *static_cast<AppState*>(*appstate);

  SDL_AppResult setupRes = setupSDL(state);
  if (setupRes != SDL_APP_CONTINUE) return setupRes;

  state.textEngine.init(state.gpu);
  state.textEngine.loadFont("assets/Helvetica.ttf", 240);

  // pre-initialize scenes
  // --> could also initialize scenes dynamically
  SDL_GPUTextureFormat scFormat = SDL_GetGPUSwapchainTextureFormat(state.gpu, state.window);
  SdfScene *sdfscn = new SdfScene(state.gpu, scFormat);
  ObjScene *objscn = new ObjScene(state.gpu, scFormat);
  state.scenes.push_back(sdfscn);
  state.scenes.push_back(objscn);

  return SDL_APP_CONTINUE;
}

// handle events
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  AppState& state = *static_cast<AppState*>(appstate);
  switch (event->type) {
    // triggers on last window close and other things. End the program.
    case SDL_EVENT_QUIT:  
      return SDL_APP_SUCCESS;
    case SDL_EVENT_WINDOW_RESIZED:
      state.sys.winSize.x = event->window.data1;
      state.sys.winSize.y = event->window.data2;
      break;
    case SDL_EVENT_KEY_DOWN:
      // quit if user hits ESC key
      if (event->key.scancode == SDL_SCANCODE_ESCAPE) {
        return SDL_APP_SUCCESS;
      }
      if (event->key.scancode == SDL_SCANCODE_1) {
        state.currentScene = 0;
      }
      if (event->key.scancode == SDL_SCANCODE_2) {
        state.currentScene = 1;
      }
      if (event->key.scancode == SDL_SCANCODE_A) {
        state.textEngine.drawGlyphToTexture(state.overlayp->tx, '&');
      }
      break;
    case SDL_EVENT_KEY_UP:
      break;
    case SDL_EVENT_MOUSE_MOTION:
      state.sys.mousePosScreenSpace = glm::vec2(event->motion.x, event->motion.y);
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
  Uint64 newTime = SDL_GetTicksNS();
  Uint64 delta = newTime - state.sys.lifetime;
  if (delta < 100001) return SDL_APP_CONTINUE;
  state.sys.lifetime = newTime;
  if (state.timeSinceLastFps > (SDL_NS_PER_SECOND / 5)) {
    state.timeSinceLastFps = 0;
    float fps = 0.0f;
    if (delta != 0) fps = SDL_NS_PER_SECOND / delta;
    char str[100];
    SDL_snprintf(str, sizeof(str), "FPS: %.2f (Scene %d)", fps, state.currentScene + 1);
  } else {
    state.timeSinceLastFps += delta;
  }

  // update objects
  if (state.scenes.size() > 0 && state.currentScene > -1) {
    SDL_AppResult res = state.scenes.at(state.currentScene)->update(state.sys);
    if (res != SDL_APP_CONTINUE) return res;
  }

  // acquire command buffer
	SDL_GPUCommandBuffer *cmdBuf = SDL_AcquireGPUCommandBuffer(state.gpu);
  SDL_InsertGPUDebugLabel(cmdBuf, "Screen Render");
	// acquire swapchain
	SDL_GPUTexture* swapchain = NULL;
	SDL_AcquireGPUSwapchainTexture(cmdBuf, state.window, &swapchain, NULL, NULL);
	if (swapchain == NULL) {
		// if swapchain == NULL, its not ready yet - skip render
		SDL_CancelGPUCommandBuffer(cmdBuf);
		return SDL_APP_CONTINUE;
	}
	// define render pass
	SDL_GPURenderPass *pass = SDL_BeginGPURenderPass(cmdBuf, new SDL_GPUColorTargetInfo {
		.texture = swapchain,
		.clear_color = SDL_FColor{ 0.02f, 0.02f, 0.08f, 1.0f },
		.load_op = SDL_GPU_LOADOP_CLEAR,
		.store_op = SDL_GPU_STOREOP_STORE,
	}, 1, NULL);

  if (state.scenes.size() > 0 && state.currentScene > -1) {
    SDL_AppResult res = state.scenes.at(state.currentScene)->render(cmdBuf, pass, swapchain);
    if (res != SDL_APP_CONTINUE) {
      SDL_CancelGPUCommandBuffer(cmdBuf);
      return res;
    }
  );
  state.overlayp->render(cmdBuf, pass, swapchain, glm::vec2(800.0f, 600.0f));

  // finish render pass
	SDL_EndGPURenderPass(pass);
	if (!SDL_SubmitGPUCommandBuffer(cmdBuf)) {
		SDL_Log("Failed to submit GPU command %s", SDL_GetError());
		return SDL_APP_FAILURE;
	};

  return SDL_APP_CONTINUE;
}

// clean up on exit
void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  AppState& state = *static_cast<AppState*>(appstate);
  SDL_Log("Closing SDL3");
  
  for (Scene* &scene : state.scenes) {
    scene->destroy();
    delete scene;
  }
  state.scenes.clear();

  state.overlayp->destroy();
  delete state.overlayp;

  state.textEngine.destroy();

  SDL_ReleaseWindowFromGPUDevice(state.gpu, state.window);
  SDL_DestroyGPUDevice(state.gpu);
  SDL_DestroySurface(state.winIcon);
  SDL_DestroyWindow(state.window);
  SDL_Quit();
}
