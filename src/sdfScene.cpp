#include "app.hpp"

using namespace App;

SdfScene::SdfScene(SDL_GPUDevice *gpu, SDL_GPUTextureFormat targetFormat) : Scene() {
  sdfPipe = new SDFPipeline(targetFormat, gpu);

  SDFObject cir1 = SDFObject::circle(glm::vec2{ 500.0f, 450.0f }, 38.0f);
  cir1.withColor(RED);
  SDFObject cir2 = SDFObject::circle(glm::vec2{ 300.0f, 200.0f }, 50.0f);
  cir2.withColor(PURPLE);
  SDFObject rect1 = SDFObject::rect(glm::vec2{ 400.0f, 200.0f }, glm::vec2{ 50.0f, 60.0f });
  rect1.withColor(modAlpha(GREEN, 0.8f));
  rect1.withRoundCorner(10.0f);
  SDFObject tri1 = SDFObject::triangle(glm::vec2{ 100.0f, 400.0f}, glm::vec2{ 220.0f, 330.0f }, glm::vec2{ 180.0f, 500.0f });
  tri1.withColor(GRAY);
  tri1.withRoundCorner(5.0f);
  objects.push_back(cir1);
  objects.push_back(cir2);
  objects.push_back(rect1);
  objects.push_back(tri1);
}

SDL_AppResult SdfScene::update(SystemUpdates const &sys) {
  screenSize = sys.winSize;
  sdfLightPos = sys.mousePosScreenSpace;

  return SDL_APP_CONTINUE;
}

SDL_AppResult SdfScene::render(SDL_GPUCommandBuffer *cmdBuf, SDL_GPURenderPass *pass, SDL_GPUTexture* screen) {
  sdfPipe->refreshObjects(objects);
  sdfPipe->render(cmdBuf, pass, screen, SDFSysData {
    .screenSize = screenSize,
    .lightPos = sdfLightPos,
    .lightColor = SDL_FColor{0.8f, 0.8f, 0.2f, 0.8f},
    .lightDist = 800.0f,
    .objCount = (Uint32)objects.size()
  });
  return SDL_APP_CONTINUE;
}

void SdfScene::destroy() {
  objects.clear();
  sdfPipe->destroy();
  delete sdfPipe;
}