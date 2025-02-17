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

## Example pipelines
- ObjectPipeline

Standard generic object pipeline for 2D/3D objects.
Objects are defined with vertices, and transformed with a standard MVP matrix.

Anti-aliasing not included.

- TextPipeline

Renders text using the SDL3_TTF library. Draws quads on the target with the appropriate
UV coordinates corresponding to a texture  atlas.

- SDFPipeline

Renders 2D shapes using SDFs instead of the standard object method.

## Installation
Compiled using g++ from the default msys2 location:
- C:/msys64/ucrt64/bin/g++.exe

glm package is installed through msys2
```
pacman -Ss glm
pacman -S {pkg_name}
```

SDL3/SDL3_ttf/SDL3_image is assumed to be manually installed at C:/Programs/SDL3
(msys2 packages prevents statically linking standard libraries)

SDL3 installation instructions from source:
(prerequisite: CMake)

1. Download SDL3 source code from https://github.com/libsdl-org/SDL
2. open terminal in source folder
3. `mkdir build`
4. `cd build`
5. `cmake .. -G "MinGW Makefiles" -DCMAKE_INSTALL_PREFIX=C:\Programs\SDL3`
6. `cmake --build . --config Release`
7. `cmake --install .`

Repeat the above for SDL3_ttf and SDL3_image

## Build
.vscode is setup to run debug mode in VSCode (F5).

Otherwise, `release.bat` is setup to compile a standalone release version.

Please note the release version requires a SDL3.dll in the same folder.

For shaders, you will need the Vulkan SDK to compile GLSL/HLSL to SPIR-V.
This is done through the `compile.bat` script.
