#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in uvec4 inBoneIDs;
layout(location = 4) in vec4 inBoneWeights;

out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform mat4 boneMatrices[100];

void main() {
    mat4 skinMatrix = boneMatrices[inBoneIDs.x] * inBoneWeights.x +
        boneMatrices[inBoneIDs.y] * inBoneWeights.y +
        boneMatrices[inBoneIDs.z] * inBoneWeights.z +
        boneMatrices[inBoneIDs.w] * inBoneWeights.w;

    vec4 skinnedPos = skinMatrix * vec4(aPos, 1.0);
    vec3 skinnedNormal = mat3(skinMatrix) * aNormal;

    vec4 worldPos = model * skinnedPos;
    gl_Position = projection * view * worldPos;

    FragPos = worldPos.xyz;
    TexCoords = aTexCoord;
    Normal = mat3(transpose(inverse(model))) * skinnedNormal;
}