#include "core/texture.hpp"
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <iostream>

bool Texture::loadFromFile(const std::string &filePath)
{
  stbi_set_flip_vertically_on_load(true); // Flip texture vertically
  unsigned char *data = stbi_load(filePath.c_str(), &width, &height, &channels, 0);
  if (!data)
  {
    std::cerr << "Failed to load texture: " << filePath << std::endl;
    return false;
  }

  // auto selects the format depending on the amount of channels
  GLenum format = GL_RGB;
  if (channels == 1)
    format = GL_RED;
  else if (channels == 3)
    format = GL_RGB;
  else if (channels == 4)
    format = GL_RGBA;

  if (textureID == 0)
    glGenTextures(1, &textureID);

  glBindTexture(GL_TEXTURE_2D, textureID);

  glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glBindTexture(GL_TEXTURE_2D, 0);

  stbi_image_free(data);
  return true;
}

void Texture::bind(unsigned int slot) const
{
  if (textureID == 0)
    return;

  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, textureID);
}

void Texture::unbind() const
{
  glBindTexture(GL_TEXTURE_2D, 0);
}