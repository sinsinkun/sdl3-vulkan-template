#pragma once

#include <SDL3/SDL.h>
#include <freetype2/ft2build.h>
#include <freetype/freetype.h>

namespace App {
  class TextEngine {
  public:
    FT_Library ftlib;
    FT_Face font;
    FT_Error init();
    FT_Error loadFont(const char *filepathname);
    void destroy();
  };
}