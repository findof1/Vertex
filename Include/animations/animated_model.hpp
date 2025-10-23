#pragma once

#include "animated_mesh.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <memory>
#include <vector>
#include <string>
#include <map>

struct BoneInfo
{
    glm::mat4 offsetMatrix;
    glm::mat4 finalTransformation;
};

class AnimatedModel
{
public:
    AnimatedModel()
    {
        defaultBoneMatrices.resize(100, glm::mat4(1.0f));
        finalBoneMatrices = std::make_shared<std::vector<glm::mat4>>(defaultBoneMatrices);
    }

    ~AnimatedModel() = default;

    static std::shared_ptr<AnimatedModel> createModelFromFile(const std::string &path, bool loadMaterials = true);

    void Draw() const;

    std::vector<std::unique_ptr<AnimatedMesh>> meshes;

    std::map<std::string, int> boneMapping;
    int boneCount = 0;

    std::vector<BoneInfo> boneInfo;
    std::vector<glm::mat4> defaultBoneMatrices;
    std::shared_ptr<std::vector<glm::mat4>> finalBoneMatrices;

    void BindAnimation(const std::shared_ptr<std::vector<glm::mat4>> &animationMatrices)
    {
        finalBoneMatrices = animationMatrices;
    }

    void UnbindAnimation()
    {
        finalBoneMatrices = std::make_shared<std::vector<glm::mat4>>(defaultBoneMatrices);
    }

    const std::vector<glm::mat4> &GetFinalBoneMatrices() const
    {
        return *finalBoneMatrices;
    }

private:
    void loadModel(const std::string &path, bool loadMaterials);
    void processNode(aiNode *node, const aiScene *scene, bool loadMaterials);
    std::unique_ptr<AnimatedMesh> processMesh(aiMesh *mesh, const aiScene *scene, bool loadMaterials);
    std::vector<AnimatedVertex> loadVertices(aiMesh *mesh);
    std::vector<unsigned int> loadIndices(aiMesh *mesh);
    void loadBones(aiMesh *mesh, std::vector<AnimatedVertex> &vertices);
    void addBoneData(AnimatedVertex &vertex, int boneID, float weight);
    std::string directory;
};