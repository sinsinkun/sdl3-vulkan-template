#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

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

// initialization of app
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
  *appstate = new AppState;
  AppState& state = *static_cast<AppState*>(*appstate);

  SDL_AppResult setupRes = setupSDL(state);
  if (setupRes != SDL_APP_CONTINUE) return setupRes;
  state.font = TTF_OpenFont("assets/Helvetica.ttf", 18);

  SDL_GPUTextureFormat scFormat = SDL_GetGPUSwapchainTextureFormat(state.gpu, state.window);
  state.overlayp = new TextPipeline(scFormat, state.gpu);
  state.sdfp = new SDFPipeline(scFormat, state.gpu);
  state.sdfpDebug = new SDFPipeline(scFormat, state.gpu);

  StringObject str1 = StringObject(state.textEngine, state.font, "FPS: 9999.00");
  StringObject str2 = StringObject(state.textEngine, state.font, "Debug: ");
  str2.color = ORANGE;
  str2.origin = Vec3{0.0f, 580.0f, 0.0f};
  state.overlayp->strings.push_back(str1);
  state.overlayp->strings.push_back(str2);

  SDFObject cir1 = SDFObject::circle(Vec2 { 500.0f, 450.0f }, 38.0f);
  cir1.withColor(RED);
  SDFObject cir2 = SDFObject::circle(Vec2 { 300.0f, 200.0f }, 50.0f);
  cir2.withColor(PURPLE);
  SDFObject rect1 = SDFObject::rect(Vec2 { 400.0f, 200.0f }, Vec2 { 50.0f, 60.0f });
  rect1.withColor(modAlpha(GREEN, 0.8f));
  rect1.withRoundCorner(10.0f);
  SDFObject tri1 = SDFObject::triangle(Vec2 { 100.0f, 400.0f}, Vec2 { 220.0f, 330.0f }, Vec2 { 180.0f, 500.0f });
  tri1.withColor(GRAY);
  tri1.withRoundCorner(5.0f);
  state.sdfObjects.push_back(cir1);
  state.sdfObjects.push_back(cir2);
  state.sdfObjects.push_back(rect1);
  state.sdfObjects.push_back(tri1);

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
      state.winSize.x = event->window.data1;
      state.winSize.y = event->window.data2;
      break;
    case SDL_EVENT_KEY_DOWN:
      // quit if user hits ESC key
      if (event->key.scancode == SDL_SCANCODE_ESCAPE) {
        return SDL_APP_SUCCESS;
      }
      if (event->key.scancode == SDL_SCANCODE_F1) {
        state.overlayp->strings[0].visible = true;
      }
      if (event->key.scancode == SDL_SCANCODE_SPACE) {
        state.sdfPosUpdate = !state.sdfPosUpdate;
      }
      break;
    case SDL_EVENT_KEY_UP:
      if (event->key.scancode == SDL_SCANCODE_F1) {
        state.overlayp->strings[0].visible = false;
      }
      break;
    case SDL_EVENT_MOUSE_MOTION:
      state.mousePosScreenSpace = Vec2(event->motion.x, event->motion.y);
      break;
    case SDL_EVENT_TEXT_INPUT:
      break;
    default:
      break;
  }
  return SDL_APP_CONTINUE;
}

float rayMarchDebug(
  std::vector<SDFObject> *cirs, Vec2 point, Vec2 target, 
  float maxDist, std::vector<SDFObject> *objs
) {
  Vec2 dir = (target - point).normalize();
  Vec2 p = point;
  float sdf = calculateSdf(p, maxDist, objs);
  float rayDist = sdf;
  // note: pipeline has a max obj count of 1000
  for (int i=0; i < 1000; i++) {
    p = p + dir * sdf;
    sdf = calculateSdf(p, maxDist, objs);
    SDFObject cir = SDFObject::circle(p, sdf);
    cir.asOutline(1.0f);
    cirs->push_back(cir);
    rayDist += sdf;
    if (rayDist > maxDist || sdf < 0.01f) break;
  }
  if (rayDist > maxDist) rayDist = maxDist;
  return rayDist;
}

// update/render loop
SDL_AppResult SDL_AppIterate(void *appstate) {
  AppState& state = *static_cast<AppState*>(appstate);

  // calculate FPS
  Uint64 newTime = SDL_GetTicksNS();
  Uint64 delta = newTime - state.lifetime;
  if (delta < 100001) return SDL_APP_CONTINUE;
  state.lifetime = newTime;
  if (state.timeSinceLastFps > (SDL_NS_PER_SECOND / 5)) {
    state.timeSinceLastFps = 0;
    float fps = 0.0f;
    if (delta != 0) fps = SDL_NS_PER_SECOND / delta;
    // SDL_Log("FPS: %.2f", fps);
    if (state.overlayp->strings.size() > 0) {
      char str[100];
      SDL_snprintf(str, sizeof(str), "FPS: %.2f", fps);
      state.overlayp->strings[0].updateText(str);
    }
  } else {
    state.timeSinceLastFps += delta;
  }

  // update sdf objects
  if (state.sdfPosUpdate) {
    // SDFObject* obj0 = &state.sdfObjects.at(0);
    // obj0->updatePosition(state.mousePosScreenSpace);
    SDFObject* obj1 = &state.sdfObjects.at(1);
    obj1->updatePosition(Vec2(
      300.0f + 100.0f * SDL_sin(0.001f * (state.lifetime / SDL_NS_PER_MS)),
      300.0f + 100.0f * SDL_cos(0.001f * (state.lifetime / SDL_NS_PER_MS))
    ));
    SDFObject* obj3 = &state.sdfObjects.at(3);
    obj3->updatePosition(Vec2(
      100.0f,
      400.0f + 100.0f * SDL_sin(0.001f * (state.lifetime / SDL_NS_PER_MS))
    ));
  }
  
  // update debug sdf objects
  Vec2 lightPos { 200.0f, 200.0f };
  std::vector<SDFObject> debugCirs;
  float sdf = calculateSdf(state.mousePosScreenSpace, 10000.0f, &state.sdfObjects);
  SDFObject sdfCir = SDFObject::circle(state.mousePosScreenSpace, SDL_fabsf(sdf));
  sdfCir.asOutline(1.0f);
  if (sdf < 0.0f) sdfCir.withColor(BLACK);
  debugCirs.push_back(sdfCir);
  SDFObject lineToLight = SDFObject::line(state.mousePosScreenSpace, lightPos, 1.0f);
  debugCirs.push_back(lineToLight);
  float distFromLight = (lightPos - state.mousePosScreenSpace).magnitude();
  float rm = rayMarchDebug(&debugCirs, state.mousePosScreenSpace, lightPos, distFromLight, &state.sdfObjects);

  // update debug text
  char str[400];
  SDL_snprintf(str, sizeof(str), "Debug | SDF: %.2f, D: %.2f, RM: %.2f", sdf, distFromLight, rm);
  state.overlayp->strings[1].updateText(str);

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
		.clear_color = SDL_FColor{ 0.1f, 0.1f, 0.2f, 1.0f },
		.load_op = SDL_GPU_LOADOP_CLEAR,
		.store_op = SDL_GPU_STOREOP_STORE,
	}, 1, NULL);

  // update render objects in sync with render
  state.sdfp->refreshObjects(state.sdfObjects);
  // start render pipelines
  state.sdfp->render(
    cmdBuf, pass, swapchain,
    SDFSysData {
      .screenSize = state.winSize,
      .lightPos = state.mousePosScreenSpace,
      .lightColor = SDL_FColor{0.8f, 0.8f, 0.2f, 0.8f},
      .lightDist = 500.0f,
      .objCount = (Uint32)state.sdfObjects.size()
    }
  );
  state.sdfpDebug->refreshObjects(debugCirs);
  state.sdfpDebug->render(
    cmdBuf, pass, swapchain,
    SDFSysData {
      .screenSize = state.winSize,
      .lightPos = Vec2(0.0f),
      .lightColor = BLACK,
      .lightDist = 0.0f,
      .objCount = (Uint32)debugCirs.size()
    }
  );
  state.overlayp->render(cmdBuf, pass, swapchain, state.winSize);

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
  state.sdfp->destroy();
  delete state.sdfp;
  state.sdfpDebug->destroy();
  delete state.sdfpDebug;
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
