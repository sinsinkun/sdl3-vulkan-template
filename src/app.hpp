#pragma once

#include <vector>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <glm/vec2.hpp>

#include "util.hpp"
#include "sdfPipeline.hpp"
#include "textPipeline.hpp"
#include "objPipeline.hpp"

namespace App {
  // scene helpers
  struct SystemUpdates {
    glm::vec2 mousePosScreenSpace = glm::vec2(0.0f);
    glm::vec2 winSize = glm::vec2(800.0f, 600.0f);
    Uint64 lifetime = 0;
  };
  class Scene {
  public:
    virtual SDL_AppResult update(SystemUpdates const &sys) {
      SDL_Log("ERR: scene update method not overwritten");
      return SDL_APP_CONTINUE;
    };
    virtual SDL_AppResult render(SDL_GPUCommandBuffer *cmdBuf, SDL_GPUTexture* screenTx) {
      SDL_Log("ERR: scene render method not overwritten");
      return SDL_APP_CONTINUE;
    };
    virtual void destroy() {
      SDL_Log("ERR: scene destroy method not overwritten");
    };
  protected:
    Scene() {};
  };
  // scenes
  class SdfScene : public Scene {
  public:
    SdfScene(SDL_GPUDevice *gpu, SDL_GPUTextureFormat targetFormat);
    SDL_AppResult update(SystemUpdates const &sys) override;
    SDL_AppResult render(SDL_GPUCommandBuffer *cmdBuf, SDL_GPUTexture* screenTx) override;
    void destroy() override;
    SDFPipeline *sdfPipe = NULL;
    glm::vec2 screenSize = glm::vec2(0.0f);
    glm::vec2 sdfLightPos = glm::vec2(0.0f);
    std::vector<SDFObject> objects;
  };
  class ObjScene : public Scene {
  public:
    ObjScene(SDL_GPUDevice *gpu, SDL_GPUTextureFormat targetFormat);
    SDL_AppResult update(SystemUpdates const &sys);
    SDL_AppResult render(SDL_GPUCommandBuffer *cmdBuf, SDL_GPUTexture* screenTx);
    void destroy();
    ObjectPipeline *objPipe = NULL;
    glm::vec2 screenSize = glm::vec2(0.0f);
  };
  // root app state
  struct AppState {
    SDL_Window *window = NULL;
    SDL_GPUDevice *gpu = NULL;
    SDL_Surface *winIcon = NULL;
    SystemUpdates sys;
    std::vector<Scene*> scenes;
    int currentScene = -1;
    // text engine
    TTF_TextEngine *textEngine = NULL;
    TTF_Font *font = NULL;
    TextPipeline *overlayp = NULL;
    // FPS debug helpers
    StringObject *fpsOverlay = NULL;
    Uint64 timeSinceLastFps = 0;
  };
}