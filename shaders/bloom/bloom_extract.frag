#version 330 core
out vec4 FragColor;
in vec2 vTexCoords;

uniform sampler2D screenTexture;
uniform float threshold; // e.g. 0.8

void main()
{
    vec3 color = texture(screenTexture, vTexCoords).rgb;
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
    vec3 bloom = brightness > threshold ? color : vec3(0.0);
    FragColor = vec4(bloom, 1.0);
}
