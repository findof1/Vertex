#pragma once

#include <glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <memory>

struct WaterVertex
{
    glm::vec3 position;
    glm::vec2 texPos;
};

struct WaterWave
{
    glm::vec2 direction = glm::vec2(1.0f, 0.0f);
    float wavelength = 10.0f;
    float amplitude = 0.3f;
    float speed = 1.0f;
    float steepness = 0.2f;
};

struct WaterMaterial
{
    glm::vec3 deepColor = glm::vec3(0.0f, 0.2f, 0.4f);
    glm::vec3 sunDir = glm::normalize(glm::vec3(-0.3f, -1.0f, 0.2f));
    glm::vec3 sunColor = glm::vec3(1.0f, 0.95f, 0.85f);
    float foamThreshold = 0.2f;
};

class Texture;

class WaterMesh
{
public:
    WaterMaterial material;
    std::vector<WaterWave> waves;

    std::shared_ptr<Texture> waterDUDV;
    std::shared_ptr<Texture> waterNormals;

    WaterMesh(int n, float size, int waves, std::shared_ptr<Texture> waterDUDV, std::shared_ptr<Texture> waterNormals);
    WaterMesh(int n, float size, const WaterMaterial &material, const std::vector<WaterWave> &waves, std::shared_ptr<Texture> waterDUDV, std::shared_ptr<Texture> waterNormals);
    ~WaterMesh();

    void Draw() const;

private:
    void generateWaterMesh(int n, float size);
    void setupMesh();

    std::vector<WaterVertex> vertices;
    std::vector<unsigned int> indices;

    unsigned int VAO, VBO, EBO;
};