#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Clipping plane, required in all vertex shaders for modules
uniform vec4 clipPlane;
uniform bool enableClip;

void main() {
    vec4 worldPos = model * vec4(aPos, 1.0);
    gl_Position = projection * view * worldPos;
    TexCoords = aTexCoord;
    Normal = normalize(mat3(transpose(inverse(model))) * aNormal);
    FragPos = vec3(model * vec4(aPos, 1.0));

    if(enableClip)
        gl_ClipDistance[0] = dot(worldPos, clipPlane);
    else
        gl_ClipDistance[0] = 1.0;
}