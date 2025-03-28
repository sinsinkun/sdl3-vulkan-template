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

  // SDL_ttf
  if (!TTF_Init()) {
    SDL_Log("Failed to initialize SDL_ttf: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  };
  state.textEngine = TTF_CreateGPUTextEngine(state.gpu);
  if (state.textEngine == NULL) {
    SDL_Log("Failed to create text engine: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  SDL_Log("Started text engine");

  // maintain a lower poll rate
  SDL_SetHint(SDL_HINT_MAIN_CALLBACK_RATE, "10000");

  return SDL_APP_CONTINUE;
}

// asset helper
struct AssetMeta {
  Uint16 id = 0;
  Sint64 size = -1;
};

void unpackAssets(AppState& state) {
  SDL_IOStream *assets = SDL_IOFromFile("build/assets.pak", "rb");
  if (assets == NULL) {
    SDL_Log("Could not read metadata for assets\n%s", SDL_GetError());
    return;
  }
  SDL_Log("Opened assets file: %d", SDL_GetIOSize(assets));
  AssetMeta assetMetas[2];
  size_t metaSize = SDL_ReadIO(assets, &assetMetas, sizeof(assetMetas));
  if (metaSize != sizeof(assetMetas)) {
    SDL_Log("Could not read metadata for assets\n%s", SDL_GetError());
    SDL_CloseIO(assets);
    return;
  } else {
    SDL_Log("Opened assets.pak");
    SDL_Log("Asset %d size - %d", assetMetas[0].id, assetMetas[0].size);
    SDL_Log("Asset %d size - %d", assetMetas[1].id, assetMetas[1].size);
  }
  // signal error??
}

// initialization of app
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
  *appstate = new AppState;
  AppState& state = *static_cast<AppState*>(*appstate);
  state.sys.kbStates = SDL_GetKeyboardState(NULL);

  SDL_AppResult setupRes = setupSDL(state);
  if (setupRes != SDL_APP_CONTINUE) return setupRes;

  SDL_GPUTextureFormat scFormat = SDL_GetGPUSwapchainTextureFormat(state.gpu, state.window);
  state.overlayp = new TextPipeline(scFormat, state.gpu);
  state.font = TTF_OpenFont("assets/Helvetica.ttf", 18);
  state.fpsOverlay = new StringObject(state.textEngine, state.font, "FPS: 9999.00");

  // pre-initialize scenes
  // --> could also initialize scenes dynamically
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
      if (event->key.scancode == SDL_SCANCODE_F1) {
        state.fpsOverlay->visible = true;
      }
      if (event->key.scancode == SDL_SCANCODE_1) {
        state.currentScene = 0;
      }
      if (event->key.scancode == SDL_SCANCODE_2) {
        state.currentScene = 1;
      }
      break;
    case SDL_EVENT_KEY_UP:
      if (event->key.scancode == SDL_SCANCODE_F1) {
        state.fpsOverlay->visible = false;
      }
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
  state.sys.deltaTime = (float)delta / (float)SDL_NS_PER_SECOND;
  if (state.timeSinceLastFps > (SDL_NS_PER_SECOND / 5)) {
    state.timeSinceLastFps = 0;
    float fps = 0.0f;
    if (delta != 0) fps = SDL_NS_PER_SECOND / delta;
    char str[100];
    SDL_snprintf(str, sizeof(str), "FPS: %.2f (Scene %d)", fps, state.currentScene + 1);
    state.fpsOverlay->updateText(str);
  } else {
    state.timeSinceLastFps += delta;
  }

  // update objects
  if (state.scenes.size() > 0 && state.currentScene > -1) {
    state.sys.mFlags = SDL_GetMouseState(NULL, NULL);
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

  // clear swapchain
  SDL_GPURenderPass *pass = SDL_BeginGPURenderPass(cmdBuf, new SDL_GPUColorTargetInfo {
		.texture = swapchain,
		.clear_color = SDL_FColor{ 0.02f, 0.02f, 0.08f, 1.0f },
		.load_op = SDL_GPU_LOADOP_CLEAR,
		.store_op = SDL_GPU_STOREOP_STORE,
	}, 1, NULL);
  SDL_EndGPURenderPass(pass);

  // render scene
  if (state.scenes.size() > 0 && state.currentScene > -1) {
    SDL_AppResult res = state.scenes.at(state.currentScene)->render(cmdBuf, swapchain);
    if (res != SDL_APP_CONTINUE) {
      SDL_CancelGPUCommandBuffer(cmdBuf);
      return res;
    }
  }

	// overlay render
  std::vector<StringObject> overlayStrs = { *state.fpsOverlay };
  state.overlayp->render(cmdBuf, NULL, swapchain, state.sys.winSize, overlayStrs);

  // end render chain
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

  TTF_DestroyText(state.fpsOverlay->ttfText);
  delete state.fpsOverlay;
  state.overlayp->destroy();
  delete state.overlayp;

  TTF_CloseFont(state.font);
  TTF_DestroyGPUTextEngine(state.textEngine);
  TTF_Quit();

  SDL_ReleaseWindowFromGPUDevice(state.gpu, state.window);
  SDL_DestroyGPUDevice(state.gpu);
  SDL_DestroySurface(state.winIcon);
  SDL_DestroyWindow(state.window);

  SDL_Quit();
}
