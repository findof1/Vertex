#include "water/water_mesh.hpp"
#include <random>
#include <glm/gtc/constants.hpp>
#include <stb_image.h>
#include <iostream>

WaterMesh::WaterMesh(int n, float size, const std::string &envPath, const WaterMaterial &material, const std::vector<WaterWave> &waves) : material(material), waves(waves)
{
    loadCubemapFromEquirectangular(envPath);
    generateWaterMesh(n, size);
    setupMesh();
}

WaterMesh::WaterMesh(int n, float size, const std::string &envPath, int waveCount)
{
    loadCubemapFromEquirectangular(envPath);
    generateWaterMesh(n, size);
    setupMesh();
    waves.reserve(waveCount);
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dirDist(0.0f, glm::two_pi<float>());
        std::uniform_real_distribution<float> wavelengthDist(5.0f, 30.0f);
        std::uniform_real_distribution<float> amplitudeDist(0.05f, 0.4f);
        std::uniform_real_distribution<float> speedDist(0.5f, 1.0f);
        std::uniform_real_distribution<float> steepnessDist(0.01f, 0.1f);

        for (int i = 0; i < waveCount / 2; i++)
        {
            float angle = dirDist(gen);
            glm::vec2 dir = glm::normalize(glm::vec2(cos(angle), sin(angle)));

            WaterWave w;
            w.direction = dir;
            w.wavelength = wavelengthDist(gen);
            w.amplitude = amplitudeDist(gen);
            w.speed = speedDist(gen);
            w.steepness = steepnessDist(gen);
            waves.push_back(w);
        }
    }

    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dirDist(0.0f, glm::two_pi<float>());
        std::uniform_real_distribution<float> wavelengthDist(1.0f, 5.0f);
        std::uniform_real_distribution<float> amplitudeDist(0.01f, 0.02f);
        std::uniform_real_distribution<float> speedDist(0.5f, 1.0f);
        std::uniform_real_distribution<float> steepnessDist(0.01f, 0.1f);

        for (int i = 0; i < waveCount / 2; i++)
        {
            float angle = dirDist(gen);
            glm::vec2 dir = glm::normalize(glm::vec2(cos(angle), sin(angle)));

            WaterWave w;
            w.direction = dir;
            w.wavelength = wavelengthDist(gen);
            w.amplitude = amplitudeDist(gen);
            w.speed = speedDist(gen);
            w.steepness = steepnessDist(gen);
            waves.push_back(w);
        }
    }
}

WaterMesh::~WaterMesh()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void WaterMesh::generateWaterMesh(int n, float size)
{
    vertices.clear();
    indices.clear();

    for (int z = 0; z <= n; ++z)
    {
        for (int x = 0; x <= n; ++x)
        {
            float u = (float)x / (float)n;
            float v = (float)z / (float)n;
            float xpos = (u - 0.5f) * size;
            float zpos = (v - 0.5f) * size;
            vertices.push_back({glm::vec3(xpos, 0.0f, zpos)});
        }
    }

    for (int z = 0; z < n; ++z)
    {
        for (int x = 0; x < n; ++x)
        {
            unsigned int i0 = z * (n + 1) + x;
            unsigned int i1 = i0 + 1;
            unsigned int i2 = i0 + (n + 1);
            unsigned int i3 = i2 + 1;

            indices.push_back(i0);
            indices.push_back(i2);
            indices.push_back(i1);
            indices.push_back(i1);
            indices.push_back(i2);
            indices.push_back(i3);
        }
    }
}

void WaterMesh::setupMesh()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // Load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(WaterVertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // Set the vertex attribute pointers
    // Vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(WaterVertex), (void *)0);

    glBindVertexArray(0);
}

void WaterMesh::Draw() const
{
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

unsigned int WaterMesh::loadCubemapFromEquirectangular(const std::string &path)
{
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    if (!data)
    {
        std::cerr << "Failed to load equirectangular texture: " << path << std::endl;
        return 0;
    }

    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);

    return textureID;
}
