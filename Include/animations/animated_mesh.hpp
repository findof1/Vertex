#pragma once

#include <glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>

struct AnimatedVertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    glm::ivec4 inBoneIDs;
    glm::vec4 inBoneWeights;
};

class AnimatedMesh
{
public:
    int textureID = -1;
    AnimatedMesh(std::vector<AnimatedVertex> vertices, std::vector<unsigned int> indices);
    ~AnimatedMesh();

    void Draw() const;

private:
    void setupMesh();

    std::vector<AnimatedVertex> vertices;
    std::vector<unsigned int> indices;

    unsigned int VAO, VBO, EBO;
};