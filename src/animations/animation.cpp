#include "animations/animation.hpp"
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

Bone::Bone(const std::string &name, int id, const aiNodeAnim *channel)
    : name(name), id(id)
{
  for (unsigned int i = 0; i < channel->mNumPositionKeys; i++)
  {
    glm::vec3 position = {channel->mPositionKeys[i].mValue.x,
                          channel->mPositionKeys[i].mValue.y,
                          channel->mPositionKeys[i].mValue.z};
    float timeStamp = (float)channel->mPositionKeys[i].mTime;
    positions.push_back({position, timeStamp});
  }

  for (unsigned int i = 0; i < channel->mNumRotationKeys; i++)
  {
    aiQuaternion quat = channel->mRotationKeys[i].mValue;
    glm::quat rotation = glm::quat(quat.w, quat.x, quat.y, quat.z);
    float timeStamp = (float)channel->mRotationKeys[i].mTime;
    rotations.push_back({rotation, timeStamp});
  }

  for (unsigned int i = 0; i < channel->mNumScalingKeys; i++)
  {
    glm::vec3 scale = {channel->mScalingKeys[i].mValue.x,
                       channel->mScalingKeys[i].mValue.y,
                       channel->mScalingKeys[i].mValue.z};
    float timeStamp = (float)channel->mScalingKeys[i].mTime;
    scales.push_back({scale, timeStamp});
  }

  numPositions = positions.size();
  numRotations = rotations.size();
  numScalings = scales.size();
}

int Bone::GetPositionIndex(float animationTime)
{
  for (int i = 0; i < numPositions - 1; ++i)
    if (animationTime < positions[i + 1].timeStamp)
      return i;
  return numPositions - 1;
}

int Bone::GetRotationIndex(float animationTime)
{
  for (int i = 0; i < numRotations - 1; ++i)
    if (animationTime < rotations[i + 1].timeStamp)
      return i;
  return numRotations - 1;
}

int Bone::GetScaleIndex(float animationTime)
{
  for (int i = 0; i < numScalings - 1; ++i)
    if (animationTime < scales[i + 1].timeStamp)
      return i;
  return numScalings - 1;
}

float Bone::GetScaleFactor(float lastTime, float nextTime, float animationTime)
{
  float scaleFactor = 0.0f;
  float midWayLength = animationTime - lastTime;
  float framesDiff = nextTime - lastTime;
  scaleFactor = midWayLength / framesDiff;
  return glm::clamp(scaleFactor, 0.0f, 1.0f);
}

void Bone::Update(float animationTime)
{
  glm::mat4 translation{1.0f}, rotation{1.0f}, scale{1.0f};

  // Interpolate positions
  if (numPositions > 1)
  {
    int p0Index = GetPositionIndex(animationTime);
    int p1Index = p0Index + 1;
    float factor = GetScaleFactor(positions[p0Index].timeStamp, positions[p1Index].timeStamp, animationTime);
    glm::vec3 finalPos = glm::mix(positions[p0Index].position, positions[p1Index].position, factor);
    translation = glm::translate(glm::mat4(1.0f), finalPos);
  }

  // Interpolate rotations
  if (numRotations > 1)
  {
    int r0Index = GetRotationIndex(animationTime);
    int r1Index = r0Index + 1;
    float factor = GetScaleFactor(rotations[r0Index].timeStamp, rotations[r1Index].timeStamp, animationTime);
    glm::quat finalRot = glm::slerp(rotations[r0Index].orientation, rotations[r1Index].orientation, factor);
    rotation = glm::toMat4(glm::normalize(finalRot));
  }

  // Interpolate scales
  if (numScalings > 1)
  {
    int s0Index = GetScaleIndex(animationTime);
    int s1Index = s0Index + 1;
    float factor = GetScaleFactor(scales[s0Index].timeStamp, scales[s1Index].timeStamp, animationTime);
    glm::vec3 finalScale = glm::mix(scales[s0Index].scale, scales[s1Index].scale, factor);
    scale = glm::scale(glm::mat4(1.0f), finalScale);
  }

  localTransform = translation * rotation * scale;
}

Animation::Animation(const std::string &path, const std::map<std::string, int> &modelBoneMapping)
{
  finalBoneMatrices = std::make_shared<std::vector<glm::mat4>>();

  Assimp::Importer importer;
  const aiScene *scene = importer.ReadFile(
      path,
      aiProcess_Triangulate |
          aiProcess_LimitBoneWeights |
          aiProcess_JoinIdenticalVertices);

  if (!scene || !scene->mRootNode || scene->mNumAnimations == 0)
  {
    std::cerr << "Error: Failed to load animation file: " << path << std::endl;
    return;
  }

  aiAnimation *animation = scene->mAnimations[0];
  duration = (float)animation->mDuration;
  ticksPerSecond = (float)(animation->mTicksPerSecond != 0 ? animation->mTicksPerSecond : 25.0f);

  // Read the hierarchy
  ReadHierarchyData(rootNode, scene->mRootNode);

  // Extract bones
  for (unsigned int i = 0; i < animation->mNumChannels; i++)
  {
    aiNodeAnim *channel = animation->mChannels[i];
    std::string boneName = channel->mNodeName.data;

    int boneID;
    if (modelBoneMapping.find(boneName) == modelBoneMapping.end())
    {
      continue;
    }
    else
    {
      boneID = modelBoneMapping.at(boneName);
    }

    // Assign unique ID per bone
    Bone bone(boneName, boneID, channel);
    bones.emplace(boneName, bone);
  }

  // Initialize final bone matrices
  finalBoneMatrices->resize(bones.size(), glm::mat4(1.0f));
}

void Animation::ReadHierarchyData(AssimpNodeData &dest, const aiNode *src)
{
  dest.name = src->mName.data;
  dest.transformation = glm::transpose(glm::make_mat4(&src->mTransformation.a1)); // Assimp uses row-major

  dest.children.reserve(src->mNumChildren);
  for (unsigned int i = 0; i < src->mNumChildren; i++)
  {
    AssimpNodeData child;
    ReadHierarchyData(child, src->mChildren[i]);
    dest.children.push_back(child);
  }
}

void Animation::Update(float deltaTime)
{
  if (bones.empty())
    return;

  currentTime += deltaTime * ticksPerSecond;
  currentTime = fmod(currentTime, duration); // loop animation

  CalculateBoneTransform(&rootNode, glm::mat4(1.0f));
}

void Animation::CalculateBoneTransform(const AssimpNodeData *node, glm::mat4 parentTransform)
{
  std::string nodeName = node->name;
  glm::mat4 nodeTransform = node->transformation;

  auto boneIt = bones.find(nodeName);
  if (boneIt != bones.end())
  {
    boneIt->second.Update(currentTime);
    nodeTransform = boneIt->second.GetLocalTransform();
  }

  glm::mat4 globalTransform = parentTransform * nodeTransform;

  // Only set if this node corresponds to a bone
  if (boneIt != bones.end())
  {
    int index = boneIt->second.GetID();
    finalBoneMatrices->at(index) = globalTransform;
  }

  for (const auto &child : node->children)
    CalculateBoneTransform(&child, globalTransform);
}