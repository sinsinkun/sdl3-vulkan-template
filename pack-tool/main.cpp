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
  std::vector<MetaData> meta;
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
    meta.push_back(metadata);
    inputs.push_back(file);
  }

  // generate output
  SDL_Log("Writing to .dat file");
  SDL_IOStream *output = SDL_IOFromFile("assets.dat", "wb");
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
  for (int i=0; i<inputs.size(); i++) {

    writeSize = SDL_WriteIO(output, inputs[i], meta[i].size);
    if (writeSize != meta[i].size) {
      SDL_Log("Failed to write to output file: %d/%d\n -> %s", writeSize, meta[i].size, SDL_GetError());
      closeFiles(inputs);
      SDL_CloseIO(output);
      return 4;
    }
    SDL_Log("Packed file (%s): %d/%d", files[i].c_str(), writeSize, meta[i].size);
  }

  // clean up
  SDL_Log("Finished packing");
  closeFiles(inputs);
  SDL_CloseIO(output);
  return 0;
}