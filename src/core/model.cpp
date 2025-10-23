#include "core/model.hpp"
#include <iostream>

std::shared_ptr<Model> Model::createModelFromFile(const std::string &path, bool loadMaterials)
{
    auto model = std::make_shared<Model>();
    model->loadModel(path, loadMaterials);
    return model;
}

void Model::loadModel(const std::string &path, bool loadMaterials)
{
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }

    directory = path.substr(0, path.find_last_of('/'));
    processNode(scene->mRootNode, scene, loadMaterials);
}

void Model::processNode(aiNode *node, const aiScene *scene, bool loadMaterials)
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene, loadMaterials));
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene, loadMaterials);
    }
}

std::unique_ptr<Mesh> Model::processMesh(aiMesh *mesh, const aiScene *scene, bool loadMaterials)
{
    std::vector<Vertex> vertices = loadVertices(mesh);
    std::vector<unsigned int> indices = loadIndices(mesh);

    auto finalMesh = std::make_unique<Mesh>(vertices, indices);
    if (loadMaterials)
    {
        finalMesh->textureID = mesh->mMaterialIndex;
    }
    return finalMesh;
}

std::vector<Vertex> Model::loadVertices(aiMesh *mesh)
{
    std::vector<Vertex> vertices;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;

        // Position
        glm::vec3 vector;
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.position = vector;

        // Normal
        if (mesh->mNormals)
        {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.normal = vector;
        }
        else
        {
            vertex.normal = glm::vec3(0.0f, 0.0f, 0.0f);
        }

        // Texture coordinates
        if (mesh->mTextureCoords[0])
        {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.texCoords = vec;
        }
        else
        {
            vertex.texCoords = glm::vec2(0.0f, 0.0f);
        }

        vertices.push_back(vertex);
    }

    return vertices;
}

std::vector<unsigned int> Model::loadIndices(aiMesh *mesh)
{
    std::vector<unsigned int> indices;

    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    return indices;
}

void Model::Draw() const
{
    for (const auto &mesh : meshes)
    {
        mesh->Draw();
    }
}