#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <string>

class Material {
public:
    Material() = default;
    virtual ~Material() = default;

    void setColor(const glm::vec3& color) { albedo = color; }
    
    // Basic material properties
    void setAlbedo(const glm::vec3& color) { albedo = color; }
    void setRoughness(float roughness) { this->roughness = roughness; }
    void setMetallic(float metallic) { this->metallic = metallic; }
    void setSpecular(const glm::vec3& specular) { this->specular = specular; }
    void setShininess(float shininess) { this->shininess = shininess; }
    
    glm::vec3 getAlbedo() const { return albedo; }
    float getRoughness() const { return roughness; }
    float getMetallic() const { return metallic; }
    glm::vec3 getSpecular() const { return specular; }
    float getShininess() const { return shininess; }

protected:
    glm::vec3 albedo{1.0f, 1.0f, 1.0f};
    float roughness{0.5f};
    float metallic{0.0f};
    glm::vec3 specular{1.0f, 1.0f, 1.0f};
    float shininess{32.0f};
};