#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include "core/texture.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>

class TextureManager;

// PBR Material Used by the PBR module for objects rendered with PBR
class PBRMaterial
{
public:
    int id;
    bool ignoreLighting = false;
    PBRMaterial() = default;
    virtual ~PBRMaterial() = default;

    void setAlbedo(const glm::vec3 &color) { albedo = color; }
    void setRoughness(float roughness) { this->roughness = roughness; }
    void setMetallic(float metallic) { this->metallic = metallic; }
    void setSpecular(const glm::vec3 &specular) { this->specular = specular; }
    void setShininess(float shininess) { this->shininess = shininess; }

    glm::vec3 getAlbedo() const { return albedo; }
    float getRoughness() const { return roughness; }
    float getMetallic() const { return metallic; }
    glm::vec3 getSpecular() const { return specular; }
    float getShininess() const { return shininess; }

    void setAlbedoMap(const std::shared_ptr<Texture> &albedoMap) { this->albedoMap = albedoMap; }
    std::shared_ptr<Texture> getAlbedoMap() const { return albedoMap; }
    bool hasAlbedoMap() const { return albedoMap != nullptr; }

    static std::vector<std::shared_ptr<PBRMaterial>> createModelMaterialsFromFile(TextureManager *textureManager, const std::string &path);

protected:
    glm::vec3 albedo{1.0f, 1.0f, 1.0f};
    float roughness{0.5f};
    float metallic{0.0f};
    glm::vec3 specular{1.0f, 1.0f, 1.0f};
    float shininess{32.0f};

    // can add more maps here in the future
    std::shared_ptr<Texture> albedoMap;
};