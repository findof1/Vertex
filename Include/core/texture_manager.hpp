#pragma once

#include <memory>
#include "texture.hpp"
#include <unordered_map>

class TextureManager
{
public:
  std::shared_ptr<Texture> load(const std::string &path)
  {
    // This makes it so if someone tries to load an already loaded texture, it returns the already loaded texture
    if (textures.find(path) != textures.end())
      return textures[path];

    auto tex = std::make_shared<Texture>();
    tex->loadFromFile(path);
    textures[path] = tex;
    return tex;
  }

  void unload(const std::string &path)
  {
    // shared_ptr will clean up if no one else references it
    textures.erase(path);
  }

  void clear()
  {
    textures.clear();
  }

private:
  std::unordered_map<std::string, std::shared_ptr<Texture>> textures;
};