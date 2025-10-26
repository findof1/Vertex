#pragma once

#include <glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>

struct WaterVertex
{
    glm::vec3 position;
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
    glm::vec3 deepColor = glm::vec3(0.0f, 0.2f, 0.35f);
    glm::vec3 sunDir = glm::normalize(glm::vec3(-0.3f, -1.0f, 0.2f));
    glm::vec3 sunColor = glm::vec3(1.0f, 0.95f, 0.85f);
    float foamThreshold = 0.8f;
    // float ior = 1.333f; // index of refraction //unused for now
};

class WaterMesh
{
public:
    WaterMaterial material;
    std::vector<WaterWave> waves;

    unsigned int envMap = 0;

    WaterMesh(int n, float size, const std::string &envPath, int waves);
    WaterMesh(int n, float size, const std::string &envPath, const WaterMaterial &material, const std::vector<WaterWave> &waves);
    ~WaterMesh();

    void Draw() const;

    void LoadEnvMap(const std::string &equirectangularPath);

private:
    void generateWaterMesh(int n, float size);
    void setupMesh();
    unsigned int loadCubemapFromEquirectangular(const std::string &path);

    std::vector<WaterVertex> vertices;
    std::vector<unsigned int> indices;

    unsigned int VAO, VBO, EBO;
};