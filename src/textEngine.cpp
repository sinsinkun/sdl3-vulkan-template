#include "textEngine.hpp"

using namespace App;

#pragma region FontCache

FT_Error FontCache::init(const FT_Library* lib, const char *filepathname) {
  FT_Error err = FT_New_Face(*lib, filepathname, 0, &face);
  if (err == FT_Err_Unknown_File_Format) {
    SDL_Log("Unknown font file format");
    return err;
  } else if (err != 0) {
    SDL_Log("Font file could not be read");
    return err;
  }
  SDL_Log("Successfully opened font file with %d glyphs", face->num_glyphs);
  FT_Set_Char_Size(face, 0, 20*64, 80, 80);
  return 0;
}

void FontCache::loadGlyph(FT_ULong c, SDL_GPUDevice *device) {
  FT_Error err = FT_Load_Char(face, c, FT_LOAD_RENDER);
  if (err != 0) {
    SDL_Log("Could not load glyph %c", c);
    return;
  }
  if (chars.count(c) == 1) {
    SDL_Log("Glyph already loaded into cache %c", c);
    return;
  }

  // todo: draw bitmap
  FT_GlyphSlot slot = face->glyph;
  FontChar fc {
    .textureId = 0,
    .size = Vec2(slot->bitmap.width, slot->bitmap.rows),
    .bearing = Vec2(slot->bitmap_left, slot->bitmap_top),
    .advance = (Uint32)slot->advance.x,
  };
  std::vector<Uint8> bitmap;
}

void FontCache::destroy() {
  FT_Done_Face(face);
}

#pragma endregion FontCache

#pragma region TextEngine

FT_Error TextEngine::init() {
  FT_Error err = FT_Init_FreeType(&ftlib);
  if (err != 0) {
    SDL_Log("Failed to init freetype");
    return err;
  }
  SDL_Log("Initialized Freetype");
  return 0;
}

FT_Error TextEngine::loadFont(const char *filepathname, SDL_GPUDevice *device) {
  FT_Error err = font.init(&ftlib, filepathname);
  font.loadGlyph('&', device);
  font.loadGlyph(0xF09F9881, device); // U+1F601: smiley face
  return err;
}

void TextEngine::destroy() {
  font.destroy();
  FT_Done_FreeType(ftlib);
}

#pragma endregion TextEngine