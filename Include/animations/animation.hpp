#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <assimp/scene.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include "animated_model.hpp"

struct KeyPosition
{
  glm::vec3 position;
  float timeStamp;
};

struct KeyRotation
{
  glm::quat orientation;
  float timeStamp;
};

struct KeyScale
{
  glm::vec3 scale;
  float timeStamp;
};

// Represents animation data for one bone
class Bone
{
public:
  Bone(const std::string &name, int id, const aiNodeAnim *channel);

  void Update(float animationTime);
  glm::mat4 GetLocalTransform() const { return localTransform; }

  const std::string &GetBoneName() const { return name; }
  int GetID() const { return id; }

private:
  std::vector<KeyPosition> positions;
  std::vector<KeyRotation> rotations;
  std::vector<KeyScale> scales;
  int numPositions;
  int numRotations;
  int numScalings;

  glm::mat4 localTransform;
  std::string name;
  int id;

  int GetPositionIndex(float animationTime);
  int GetRotationIndex(float animationTime);
  int GetScaleIndex(float animationTime);
  float GetScaleFactor(float lastTime, float nextTime, float animationTime);
};

struct AssimpNodeData
{
  glm::mat4 transformation;
  std::string name;
  std::vector<AssimpNodeData> children;
};

class Animation
{
public:
  Animation(const std::string &path, const std::map<std::string, int> &modelBoneMapping);
  float GetDuration() const { return duration; }
  float GetTicksPerSecond() const { return ticksPerSecond; }
  std::shared_ptr<std::vector<glm::mat4>> GetFinalBoneMatrices() const { return finalBoneMatrices; }

  void Update(float deltaTime);

  std::shared_ptr<std::vector<glm::mat4>> finalBoneMatrices;

private:
  void ReadHierarchyData(AssimpNodeData &dest, const aiNode *src);
  void CalculateBoneTransform(const AssimpNodeData *node, glm::mat4 parentTransform);

  float duration;
  float ticksPerSecond;
  std::unordered_map<std::string, Bone> bones;
  AssimpNodeData rootNode;
  float currentTime = 0.0f;
};