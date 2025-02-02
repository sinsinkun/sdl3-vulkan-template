#include "textEngine.hpp"

using namespace App;

FT_Error TextEngine::init() {
  FT_Error err = FT_Init_FreeType(&ftlib);
  if (err != 0) {
    SDL_Log("Failed to init freetype");
    return err;
  }
  SDL_Log("Initialized Freetype");
  return 0;
}

FT_Error TextEngine::loadFont(const char *filepathname) {
  FT_Error err = FT_New_Face(ftlib, filepathname, 0, &font);
  if (err == FT_Err_Unknown_File_Format) {
    SDL_Log("Unknown font file format");
    return err;
  } else if (err != 0) {
    SDL_Log("Font file could not be read");
    return err;
  }
  SDL_Log("Successfully opened font file with %d glyphs", font->num_glyphs);
  FT_Set_Char_Size(font, 0, 20*64, 80, 80);
  return 0;
}

void TextEngine::destroy() {
  FT_Done_Face(font);
}