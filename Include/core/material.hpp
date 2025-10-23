#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include "texture.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>

class TextureManager;

// Basic Material Used by the core module for solid-color objects
class Material
{
public:
    int id;
    Material() = default;
    virtual ~Material() = default;

    void setAlbedo(const glm::vec3 &newAlbedo) { albedo = newAlbedo; }
    glm::vec3 getAlbedo() const { return albedo; }

    void setTexture(const std::shared_ptr<Texture> &tex) { texture = tex; }
    std::shared_ptr<Texture> getTexture() const { return texture; }
    bool hasTexture() const { return texture != nullptr; }

    static std::vector<std::shared_ptr<Material>> createModelMaterialsFromFile(TextureManager *textureManager, const std::string &path);

protected:
    glm::vec3 albedo{1.0f, 1.0f, 1.0f};
    std::shared_ptr<Texture> texture;
};