#include <vector>
#include <string>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

struct MetaData {
  Uint16 id = 0;
  Sint64 size = -1;
};

void closeFiles(std::vector<SDL_IOStream*> input) {
  for (SDL_IOStream *fn : input) {
    SDL_CloseIO(fn);
  }
}

int main(int argc, char* argv[]) {
  SDL_Log("Opening files");

  std::vector<std::string> files = { "assets/icon.png", "assets/Helvetica.ttf" };
  MetaData meta[files.size()];
  std::vector<SDL_IOStream*> inputs;

  // open up files
  Uint16 id = 0;
  for (std::string fn : files) {
    SDL_IOStream *file = SDL_IOFromFile(fn.c_str(), "rb");
    if (file == NULL) {
      SDL_Log("Could not open file: %s", SDL_GetError());
      closeFiles(inputs);
      return 1;
    }
    Sint64 fileSize = SDL_GetIOSize(file);
    id++;
    MetaData metadata = { id, fileSize };
    meta[id - 1] = metadata;
    inputs.push_back(file);
  }

  // generate output
  SDL_Log("Writing to .pak file");
  SDL_IOStream *output = SDL_IOFromFile("build/assets.pak", "wb");
  if (output == NULL) {
    SDL_Log("Failed to create output file: %s", SDL_GetError());
    closeFiles(inputs);
    return 3;
  }

  // write metadata into output
  size_t writeSize = SDL_WriteIO(output, &meta, sizeof(meta));
  if (writeSize != sizeof(meta)) {
    SDL_Log("Failed to write to output file: %d/%d", writeSize, sizeof(meta));
    closeFiles(inputs);
    SDL_CloseIO(output);
    return 4;
  }
  SDL_Log("Packed metadata: %d/%d", writeSize, sizeof(meta));

  // write actual data
  char buffer[10240];
  size_t bytesRead = 0;
  size_t bytesWritten = 0;
  int counter = 0;
  for (int i=0; i<inputs.size(); i++) {
    // copy data 10kb at a time to bypass SDL_WriteIO size limitations
    bytesRead = SDL_ReadIO(inputs[i], buffer, sizeof(buffer));
    while (bytesRead > 0) {
      bytesWritten = SDL_WriteIO(output, buffer, bytesRead);
      if (bytesRead != bytesWritten) {
        SDL_Log("Failed to write to output file: %d/%d\n -> %s", bytesWritten, meta[i].size, SDL_GetError());
        closeFiles(inputs);
        SDL_CloseIO(output);
      }
      bytesRead = SDL_ReadIO(inputs[i], buffer, sizeof(buffer));
      counter++;
    }
    SDL_Log("Packed file (%s): %d/%d - %d iterations", files[i].c_str(), bytesWritten, meta[i].size, counter);
    bytesRead = 0;
    bytesWritten = 0;
    counter = 0;
  }

  // clean up
  SDL_Log("Finished packing");
  closeFiles(inputs);
  SDL_CloseIO(output);
  return 0;
}