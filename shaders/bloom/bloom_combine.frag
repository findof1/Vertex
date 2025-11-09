#version 330 core
out vec4 FragColor;
in vec2 vTexCoords;

uniform sampler2D sceneTexture; // original rendered scene
uniform sampler2D bloomTexture; // blurred bright texture
uniform float bloomStrength;    // 0.0–1.0

void main()
{
    vec3 scene = texture(sceneTexture, vTexCoords).rgb;
    vec3 bloom = texture(bloomTexture, vTexCoords).rgb;
    vec3 color = scene + bloom * bloomStrength;
    FragColor = vec4(color, 1.0);
}
