#include "pbr/pbr_material.hpp"
#include "core/texture_manager.hpp"
#include <iostream>

void processNode(TextureManager *textureManager, aiNode *node, const aiScene *scene, std::vector<std::shared_ptr<PBRMaterial>> &materials, const std::string &directory)
{
  // Process all meshes in this node
  for (unsigned int i = 0; i < node->mNumMeshes; i++)
  {
    aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
    aiMaterial *aiMat = scene->mMaterials[mesh->mMaterialIndex];

    // Create Material for this mesh
    auto mat = std::make_shared<PBRMaterial>();

    aiColor3D color;
    float value;

    if (AI_SUCCESS == aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, color))
      mat->setAlbedo(glm::vec3(color.r, color.g, color.b));

    if (AI_SUCCESS == aiMat->Get(AI_MATKEY_COLOR_SPECULAR, color))
      mat->setSpecular(glm::vec3(color.r, color.g, color.b));

    if (AI_SUCCESS == aiMat->Get(AI_MATKEY_SHININESS, value))
    {
      mat->setShininess(value);
      mat->setRoughness(1.0f - value / 128.0f); // simple approximation
    }

    mat->setMetallic(0.0f);

    if (aiMat->GetTextureCount(aiTextureType_DIFFUSE) > 0)
    {
      aiString str;
      aiMat->GetTexture(aiTextureType_DIFFUSE, 0, &str);
      auto tex = textureManager->load(directory + "/" + str.C_Str());
      mat->setAlbedoMap(tex);
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

std::vector<std::shared_ptr<PBRMaterial>> PBRMaterial::createModelMaterialsFromFile(TextureManager *textureManager, const std::string &path)
{
  Assimp::Importer importer;
  const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
  {
    std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
    return {};
  }

  std::string directory = path.substr(0, path.find_last_of('/'));

  std::vector<std::shared_ptr<PBRMaterial>> materials;
  processNode(textureManager, scene->mRootNode, scene, materials, directory);
  return materials;
}
