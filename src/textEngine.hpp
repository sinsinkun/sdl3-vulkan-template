#pragma once

#include <string>
#include <map>
#include <SDL3/SDL.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "util.hpp"

namespace App {
  struct FontChar {
    Uint8 *buffer = NULL;
    Uint32 bufferSize;
    // size in bytes
    Uint32 pxWidth;
    Uint32 pxHeight;
    // offset from baseline to left/top
    int offsetLeft;
    int offsetTop;
    // offset to next glyph
    Uint32 advance;
  };
  class FontCache {
  public:
    FT_Face face;
    std::map<FT_ULong, FontChar> chars;
    FT_Error init(const FT_Library* lib, const char* filepathname, int fontSize);
    void cacheGlyph(FT_ULong c);
    FontChar* getGlyph(FT_ULong c);
    void destroy();
  };
  class TextEngine {
  public:
    FT_Library ftlib;
    SDL_GPUDevice *device = NULL;
    SDL_GPUTexture *screenTx = NULL;
    FT_Error init(SDL_GPUDevice *device);
    FT_Error loadFont(const char *filepathname, int fontSize);
    void drawGlyphToTexture(SDL_GPUTexture *tx, char c);
    FontCache font;
    void destroy();
  };
}