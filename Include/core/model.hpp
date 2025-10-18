#pragma once

#include "mesh.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <memory>
#include <vector>
#include <string>

class Model {
public:
    Model() = default;
    ~Model() = default;

    static std::shared_ptr<Model> createModelFromFile(const std::string& path);
    
    void Draw() const;
    
    std::vector<std::unique_ptr<Mesh>> meshes;

private:
    void loadModel(const std::string& path);
    void processNode(aiNode* node, const aiScene* scene);
    std::unique_ptr<Mesh> processMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<Vertex> loadVertices(aiMesh* mesh);
    std::vector<unsigned int> loadIndices(aiMesh* mesh);

    std::string directory;
};