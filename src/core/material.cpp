#include "core/material.hpp"
#include <iostream>
#include "core/texture_manager.hpp"

void processNode(TextureManager *textureManager, aiNode *node, const aiScene *scene, std::vector<std::shared_ptr<Material>> &materials, const std::string &directory)
{
  // Process all meshes in this node
  for (unsigned int i = 0; i < node->mNumMeshes; i++)
  {
    aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
    aiMaterial *aiMat = scene->mMaterials[mesh->mMaterialIndex];

    // Create Material for this mesh
    auto mat = std::make_shared<Material>();

    if (aiMat->GetTextureCount(aiTextureType_DIFFUSE) > 0)
    {
      aiString str;
      aiMat->GetTexture(aiTextureType_DIFFUSE, 0, &str);
      auto tex = textureManager->load(directory + "/" + str.C_Str());
      mat->setTexture(tex);
    }

    mat->id = mesh->mMaterialIndex;

    materials.push_back(mat);
  }

  // Process child nodes recursively
  for (unsigned int i = 0; i < node->mNumChildren; i++)
  {
    processNode(textureManager, node->mChildren[i], scene, materials, directory);
  }
}

std::vector<std::shared_ptr<Material>> Material::createModelMaterialsFromFile(TextureManager *textureManager, const std::string &path)
{
  Assimp::Importer importer;
  const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
  {
    std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
    return {};
  }

  std::string directory = path.substr(0, path.find_last_of('/'));

  std::vector<std::shared_ptr<Material>> materials;
  processNode(textureManager, scene->mRootNode, scene, materials, directory);
  return materials;
}
