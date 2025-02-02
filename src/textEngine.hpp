#pragma once

#include <vector>
#include <map>
#include <SDL3/SDL.h>
#include <ft2build.h>
#include <freetype/freetype.h>
#include "util.hpp"

namespace App {
  struct FontChar {
    Uint32 textureId;
    Vec2 size;
    Vec2 bearing;
    Uint32 advance;
  };
  class FontCache {
  public:
    FT_Face face;
    std::map<char, FontChar> chars;
    FT_Error init(const FT_Library* lib, const char* filepathname);
    void loadGlyph(FT_ULong c, SDL_GPUDevice *device);
    void destroy();
  };
  class TextEngine {
  public:
    FT_Library ftlib;
    FT_Error init();
    FT_Error loadFont(const char *filepathname, SDL_GPUDevice *device);
    FontCache font;
    void destroy();
  };
}