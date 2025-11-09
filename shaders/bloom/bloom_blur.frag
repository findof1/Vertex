#version 330 core
out vec4 FragColor;
in vec2 vTexCoords;

uniform sampler2D screenTexture;
uniform float texelWidth;  // 1.0 / screenWidth
uniform float texelHeight; // 1.0 / screenHeight
uniform float blurScale;
uniform int horizontal;

void main()
{
    vec2 tex_offset = vec2(texelWidth, texelHeight) * blurScale; 
    vec3 result = texture(screenTexture, vTexCoords).rgb * 0.227027;

    if (horizontal == 1) {
    result += texture(screenTexture, vTexCoords + vec2(tex_offset.x * 1.0, 0.0)).rgb * 0.1945946;
    result += texture(screenTexture, vTexCoords - vec2(tex_offset.x * 1.0, 0.0)).rgb * 0.1945946;
    result += texture(screenTexture, vTexCoords + vec2(tex_offset.x * 2.0, 0.0)).rgb * 0.1216216;
    result += texture(screenTexture, vTexCoords - vec2(tex_offset.x * 2.0, 0.0)).rgb * 0.1216216;
    result += texture(screenTexture, vTexCoords + vec2(tex_offset.x * 3.0, 0.0)).rgb * 0.054054;
    result += texture(screenTexture, vTexCoords - vec2(tex_offset.x * 3.0, 0.0)).rgb * 0.054054;
    result += texture(screenTexture, vTexCoords + vec2(tex_offset.x * 4.0, 0.0)).rgb * 0.016216;
    result += texture(screenTexture, vTexCoords - vec2(tex_offset.x * 4.0, 0.0)).rgb * 0.016216;
} else {
    result += texture(screenTexture, vTexCoords + vec2(0.0, tex_offset.y * 1.0)).rgb * 0.1945946;
    result += texture(screenTexture, vTexCoords - vec2(0.0, tex_offset.y * 1.0)).rgb * 0.1945946;
    result += texture(screenTexture, vTexCoords + vec2(0.0, tex_offset.y * 2.0)).rgb * 0.1216216;
    result += texture(screenTexture, vTexCoords - vec2(0.0, tex_offset.y * 2.0)).rgb * 0.1216216;
    result += texture(screenTexture, vTexCoords + vec2(0.0, tex_offset.y * 3.0)).rgb * 0.054054;
    result += texture(screenTexture, vTexCoords - vec2(0.0, tex_offset.y * 3.0)).rgb * 0.054054;
    result += texture(screenTexture, vTexCoords + vec2(0.0, tex_offset.y * 4.0)).rgb * 0.016216;
    result += texture(screenTexture, vTexCoords - vec2(0.0, tex_offset.y * 4.0)).rgb * 0.016216;
}

    FragColor = vec4(result, 1.0);
}
