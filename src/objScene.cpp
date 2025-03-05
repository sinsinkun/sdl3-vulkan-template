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

  int obj1id = objPipe->uploadObject(tube(80.0f, 40.0f, 100.0f, 18));
  RenderObject &obj1 = objPipe->getObject(obj1id);
  obj1.albedo = CYAN;
  obj1.rotAxis = glm::vec3(0.0f, 1.0f, 0.0f);
  obj1.rotAngleRad = 0.5f;

  int obj2id = objPipe->uploadObject(cube(150.0f, 100.0f, 100.0f));
  RenderObject &obj2 = objPipe->getObject(obj2id);
  obj2.albedo = GREEN;
  obj2.pos = glm::vec3(200.0f, -200.0f, -100.0f);
  obj2.rotAxis = glm::vec3(0.0f, 1.0f, 0.0f);
  obj2.rotAngleRad = 0.5f;
}

SDL_AppResult ObjScene::update(SystemUpdates const &sys) {

  // resize if necessary
  if (sys.winSize.x != screenSize.x || sys.winSize.y != screenSize.y) {
    objPipe->resizeScreen((Uint32)sys.winSize.x, (Uint32)sys.winSize.y);
    screenSize = sys.winSize;
  }
  objPipe->updateCamera(RenderCamera {
    .perspective = usePerspective,
    .viewWidth = sys.winSize.x,
    .viewHeight = sys.winSize.y,
    .fovY = degToRad(60.0f),
  });

  if (getMouseBtnClicked(sys.mFlags, SDL_BUTTON_LEFT)) usePerspective = true;
  if (getMouseBtnClicked(sys.mFlags, SDL_BUTTON_RIGHT)) usePerspective = false;

  RenderObject &obj1 = objPipe->getObject(0);
  RenderObject &obj2 = objPipe->getObject(1);
  if (sys.kbStates[SDL_SCANCODE_LEFT] || sys.kbStates[SDL_SCANCODE_A]) {
    obj1.pos.x -= 100.0f * sys.deltaTime;
    obj2.rotAngleRad -= 2.0f * sys.deltaTime;
  }
  if (sys.kbStates[SDL_SCANCODE_RIGHT] || sys.kbStates[SDL_SCANCODE_D]) {
    obj1.pos.x += 100.0f * sys.deltaTime;
    obj2.rotAngleRad += 2.0f * sys.deltaTime;
  }
  if (sys.kbStates[SDL_SCANCODE_UP] || sys.kbStates[SDL_SCANCODE_W]) obj1.pos.y += 100.0f * sys.deltaTime;
  if (sys.kbStates[SDL_SCANCODE_DOWN] || sys.kbStates[SDL_SCANCODE_S]) obj1.pos.y -= 100.0f * sys.deltaTime;
  if (sys.kbStates[SDL_SCANCODE_Q]) obj1.pos.z += 100.0f * sys.deltaTime;
  if (sys.kbStates[SDL_SCANCODE_E]) obj1.pos.z -= 100.0f * sys.deltaTime;

  if (sys.kbStates[SDL_SCANCODE_SPACE]) SDL_Log("pos: (%f, %f, %f)", obj1.pos.x, obj1.pos.y, obj1.pos.z);

  return SDL_APP_CONTINUE;
}

SDL_AppResult ObjScene::render(SDL_GPUCommandBuffer *cmdBuf, SDL_GPUTexture* screen) {
  objPipe->render(cmdBuf, screen, LightMaterial {});
  return SDL_APP_CONTINUE;
}

void ObjScene::destroy() {
  objPipe->destroy();
  delete objPipe;
}