# SDL3 Template

Simple setup for SDL3 with new GPU API using vulkan.

Uses the callback architecture instead of the regular SDL_main setup.
Can be conceptualized as the following:

```cpp
int main(int argc, char *argv[]) {
  SDL_AppInit();
  bool exit = false;
  SDL_Event event;
  while (!exit) {
    while (SDL_PollEvent(&event)) {
      SDL_AppEvent();
    }
    SDL_AppIterate();
  }
  SDL_AppQuit();
  return 0;
}
```

## Installation
Compiled using g++ from the default msys2 location:
- C:/msys64/ucrt64/bin/g++.exe

SDL3 is assumed to be installed through msys2

## Build
.vscode is setup to run debug mode in VSCode (F5).

Otherwise, release.bat is setup to compile a standalone release version.

Please note the release version requires a SDL3.dll in the same folder.