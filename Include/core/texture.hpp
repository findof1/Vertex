#pragma once

#include <string>
#include <memory>

class Texture
{
public:
  // returns false if it failed, true if succeeded
  bool loadFromFile(const std::string &filePath);

  // You can only call this for each slot once per draw command
  void bind(unsigned int slot = 0) const;

  void unbind() const;

private:
  unsigned int textureID = 0;
  int width = 0;
  int height = 0;
  int channels = 0;

private:
};