#include "app.hpp"

using namespace App;

ObjScene::ObjScene(SDL_GPUDevice *gpu, SDL_GPUTextureFormat targetFormat) : Scene() {
  objPipe = new ObjectPipeline(targetFormat, gpu, PT_Tri, SDL_GPU_CULLMODE_BACK, 800, 600);
  objPipe->updateCamera(RenderCamera {
    .perspective = true,
    .viewWidth = 800.0f,
    .viewHeight = 600.0f,
    .fovY = degToRad(60.0f),
  });

  int obj1id = objPipe->uploadObject(sphere(10.0f, 24, 18));
  RenderObject &obj1 = objPipe->getObject(obj1id);
  obj1.albedo = CYAN;
  int obj2id = objPipe->uploadObject(cube(15.0f, 12.0f, 15.0f));
  RenderObject &obj2 = objPipe->getObject(obj2id);
  obj2.albedo = GREEN;
}

SDL_AppResult ObjScene::update(SystemUpdates const &sys) {

  // resize if necessary
  if (sys.winSize.x != screenSize.x || sys.winSize.y != screenSize.y) {
    objPipe->resizeScreen((Uint32)sys.winSize.x, (Uint32)sys.winSize.y);
    objPipe->updateCamera(RenderCamera {
      .perspective = true,
      .viewWidth = sys.winSize.x,
      .viewHeight = sys.winSize.y,
      .fovY = degToRad(60.0f),
    });
    screenSize = sys.winSize;
  }

  RenderObject &obj1 = objPipe->getObject(0);
  obj1.pos = glm::vec3(
    (sys.winSize.x / 2.0f - sys.mousePosScreenSpace.x) / 10.0f,
    (sys.winSize.y / 2.0f - sys.mousePosScreenSpace.y) / 10.0f,
    60.0f
  );

  RenderObject &obj2 = objPipe->getObject(1);
  obj2.pos = glm::vec3(
    (sys.mousePosScreenSpace.x - sys.winSize.x / 2.0f) / 10.0f,
    (sys.mousePosScreenSpace.y - sys.winSize.y / 2.0f) / 10.0f,
    60.0f
  );

  return SDL_APP_CONTINUE;
}

SDL_AppResult ObjScene::render(SDL_GPUCommandBuffer *cmdBuf, SDL_GPUTexture* screen) {
  objPipe->render(cmdBuf, screen, LightMaterial {
    .lightColor = SDL_FColor{0.8f, 0.8f, 0.8f, 1.0f},
    .lightPos = glm::vec3(0.0f, 10.0f, 0.0f),
    .ambientIntensity = 0.4f,
    .specularIntensity = 0.6f,
  });
  return SDL_APP_CONTINUE;
}

void ObjScene::destroy() {
  objPipe->destroy();
  delete objPipe;
}