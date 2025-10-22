#include "animations/animated_model.hpp"
#include <iostream>

std::shared_ptr<AnimatedModel> AnimatedModel::createModelFromFile(const std::string &path)
{
    auto model = std::make_shared<AnimatedModel>();
    model->loadModel(path);
    return model;
}

void AnimatedModel::loadModel(const std::string &path)
{
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }

    directory = path.substr(0, path.find_last_of('/'));
    processNode(scene->mRootNode, scene);
}

void AnimatedModel::processNode(aiNode *node, const aiScene *scene)
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

std::unique_ptr<AnimatedMesh> AnimatedModel::processMesh(aiMesh *mesh, const aiScene *scene)
{
    std::vector<AnimatedVertex> vertices = loadVertices(mesh);
    std::vector<unsigned int> indices = loadIndices(mesh);

    loadBones(mesh, vertices);

    return std::make_unique<AnimatedMesh>(vertices, indices);
}

glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4 &from)
{
    glm::mat4 to;
    to[0][0] = from.a1;
    to[1][0] = from.a2;
    to[2][0] = from.a3;
    to[3][0] = from.a4;
    to[0][1] = from.b1;
    to[1][1] = from.b2;
    to[2][1] = from.b3;
    to[3][1] = from.b4;
    to[0][2] = from.c1;
    to[1][2] = from.c2;
    to[2][2] = from.c3;
    to[3][2] = from.c4;
    to[0][3] = from.d1;
    to[1][3] = from.d2;
    to[2][3] = from.d3;
    to[3][3] = from.d4;
    return to;
}

void AnimatedModel::loadBones(aiMesh *mesh, std::vector<AnimatedVertex> &vertices)
{
    for (unsigned int i = 0; i < mesh->mNumBones; i++)
    {
        std::string boneName(mesh->mBones[i]->mName.data);

        int boneIndex = 0;
        if (boneMapping.find(boneName) == boneMapping.end())
        {
            boneIndex = boneCount;
            boneMapping[boneName] = boneCount;

            BoneInfo bi;
            bi.offsetMatrix = aiMatrix4x4ToGlm(mesh->mBones[i]->mOffsetMatrix);
            boneInfo.push_back(bi);
            boneCount++;
        }
        else
        {
            boneIndex = boneMapping[boneName];
        }

        // Assign bone weights to vertices
        for (unsigned int j = 0; j < mesh->mBones[i]->mNumWeights; j++)
        {
            unsigned int vertexID = mesh->mBones[i]->mWeights[j].mVertexId;
            float weight = mesh->mBones[i]->mWeights[j].mWeight;
            addBoneData(vertices[vertexID], boneIndex, weight);
        }
    }
}

void AnimatedModel::addBoneData(AnimatedVertex &vertex, int boneID, float weight)
{
    for (int i = 0; i < 4; i++)
    {
        if (vertex.inBoneWeights[i] == 0.0f)
        {
            vertex.inBoneIDs[i] = boneID;
            vertex.inBoneWeights[i] = weight;
            return;
        }
    }
}

std::vector<AnimatedVertex> AnimatedModel::loadVertices(aiMesh *mesh)
{
    std::vector<AnimatedVertex> vertices;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        AnimatedVertex vertex;

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

std::vector<unsigned int> AnimatedModel::loadIndices(aiMesh *mesh)
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

void AnimatedModel::Draw() const
{
    for (const auto &mesh : meshes)
    {
        mesh->Draw();
    }
}