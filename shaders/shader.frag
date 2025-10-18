#version 330 core
in vec2 TexCoords;
out vec4 FragColor;

uniform vec3 materialAlbedo;      
uniform sampler2D albedoMap;    
uniform bool useAlbedoMap;

void main() {
    vec3 color = materialAlbedo;

    if (useAlbedoMap)
        color = texture(albedoMap, TexCoords).rgb;

    FragColor = vec4(color, 1.0);
}