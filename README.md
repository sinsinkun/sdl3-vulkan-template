# SDL3 Template

Simple setup for SDL3 with new GPU API using vulkan.

Uses the callback architecture instead of the regular SDL_main setup.
Can be conceptualized as the following:

```cpp
int main(int argc, char *argv[]) {
  SDL_Result result = SDL_AppInit();
  SDL_Event event;
  while (result == SDL_APP_CONTINUE) {
    while (SDL_PollEvent(&event)) {
      result = SDL_AppEvent(&event);
    }
    result = SDL_AppIterate();
  }
  SDL_AppQuit(result);
  return 0;
}
```

## Installation
Compiled using g++ from the default msys2 location:
- C:/msys64/ucrt64/bin/g++.exe

SDL3 is assumed to be manually installed at C:/Programs/SDL3
(msys2 SDL3 package can't be linked statically)

## Build
.vscode is setup to run debug mode in VSCode (F5).

Otherwise, release.bat is setup to compile a standalone release version.

Please note the release version requires a SDL3.dll in the same folder.