#version 330 core
out vec4 FragColor;
in vec2 vTexCoords;

uniform sampler2D screenTexture;
uniform float time;       // pass elapsed time for animated noise
uniform float noiseAmount; // controls overall noise strength

// Simple random generator based on UV + time
float random(vec2 co)
{
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{
    vec3 color = texture(screenTexture, vTexCoords).rgb;

    // Compute luminance for adaptive noise (darker areas get more noise)
    float lum = dot(color, vec3(0.299, 0.587, 0.114));

    // Create animated random noise
    float noise = random(vTexCoords * vec2(1280.0, 720.0) + time * 50.0);

    // Adjust noise intensity based on luminance (more visible in dark areas)
    float intensity = mix(1.0, 0.3, lum);

    // Combine base color with noise
    vec3 noisyColor = color + (noise - 0.5) * noiseAmount * intensity;

    // Optional: slight desaturation to simulate camera grain look
    noisyColor = mix(noisyColor, vec3(dot(noisyColor, vec3(0.333))), 0.1);

    FragColor = vec4(noisyColor, 1.0);
}
