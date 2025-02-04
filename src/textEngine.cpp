#include "textEngine.hpp"

using namespace App;

#pragma region FontCache

FT_Error FontCache::init(const FT_Library* lib, const char *filepathname, int fontSize) {
  FT_Error err = FT_New_Face(*lib, filepathname, 0, &face);
  if (err == FT_Err_Unknown_File_Format) {
    SDL_Log("Unknown font file format");
    return err;
  } else if (err != 0) {
    SDL_Log("Font file could not be read");
    return err;
  }
  SDL_Log("Successfully opened font file with %d glyphs", face->num_glyphs);
  FT_Set_Char_Size(face, 0, fontSize*64, 80, 80);
  return 0;
}

void FontCache::cacheGlyph(FT_ULong c) {
  FT_Error err = FT_Load_Char(face, c, FT_LOAD_RENDER);
  if (err != 0) {
    SDL_Log("Could not load glyph %c", c);
    return;
  }
  if (chars.count(c) == 1) {
    SDL_Log("Glyph already loaded into cache %c", c);
    return;
  }

  FT_GlyphSlot slot = face->glyph;
  FontChar fc {
    .buffer = slot->bitmap.buffer,
    .bufferSize = slot->bitmap.width * slot->bitmap.rows,
    .pxWidth = slot->bitmap.width,
    .pxHeight = slot->bitmap.rows,
    .offsetLeft = slot->bitmap_left,
    .offsetTop = slot->bitmap_top,
    .advance = (Uint32)(slot->advance.x / 64),
  };
  SDL_Log("Cached glyph %c: (%d, %d, %d, %d, %d)", 
    c, fc.pxHeight, fc.pxHeight, fc.offsetLeft, fc.offsetTop, fc.advance
  );
  chars.insert(std::pair<FT_ULong, FontChar>(c, fc));
}

FontChar* FontCache::getGlyph(FT_ULong c) {
  if (chars.count(c) == 1) {
    return &chars.at(c);
  }
  cacheGlyph(c);
  return &chars.at(c);
}

void FontCache::destroy() {
  FT_Done_Face(face);
}

#pragma endregion FontCache

#pragma region TextEngine

FT_Error TextEngine::init(SDL_GPUDevice *device) {
  this->device = device;
  FT_Error err = FT_Init_FreeType(&ftlib);
  if (err != 0) {
    SDL_Log("Failed to init freetype");
    return err;
  }
  SDL_Log("Initialized Freetype");

  return 0;
}

FT_Error TextEngine::loadFont(const char *filepathname, int fontSize) {
  FT_Error err = font.init(&ftlib, filepathname, fontSize);
  // font.cacheGlyph('&');
  // font.cacheGlyph(0xF09F9881); // U+1F601: smiley face
  return err;
}

void TextEngine::drawGlyphToTexture(SDL_GPUTexture *tx, char c) {
  if (tx == NULL) {
    SDL_Log("No texture provided to draw onto");
    return;
  }
  // grab glyph from font cache
  FontChar* ch = font.getGlyph(c);
  SDL_GPUTransferBuffer *transBuf = SDL_CreateGPUTransferBuffer(
    device,
    new SDL_GPUTransferBufferCreateInfo {
      .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
      .size = ch->bufferSize
    }
  );
  Uint8* transBufPtr = static_cast<Uint8*>(SDL_MapGPUTransferBuffer(device, transBuf, false));
  SDL_memcpy(transBufPtr, ch->buffer, ch->bufferSize);
  SDL_UnmapGPUTransferBuffer(device, transBuf);

  SDL_GPUCommandBuffer *cmdBuf = SDL_AcquireGPUCommandBuffer(device);
  SDL_GPUCopyPass *pass = SDL_BeginGPUCopyPass(cmdBuf);

  SDL_UploadToGPUTexture(
    pass,
    new SDL_GPUTextureTransferInfo {
      .transfer_buffer = transBuf,
      .offset = 0,
      .pixels_per_row = ch->pxWidth,
      .rows_per_layer = ch->pxHeight,
    },
    new SDL_GPUTextureRegion {
      .texture = tx,
      .x = 5,
      .y = 5,
      .w = ch->pxWidth,
      .h = ch->pxHeight,
      .d = 1,
    },
    false
  );

  SDL_EndGPUCopyPass(pass);
  SDL_SubmitGPUCommandBuffer(cmdBuf);
  SDL_ReleaseGPUTransferBuffer(device, transBuf);
}

void TextEngine::destroy() {
  font.destroy();
  FT_Done_FreeType(ftlib);
}

#pragma endregion TextEngine