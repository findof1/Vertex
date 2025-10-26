#version 330 core
in vec3 vWorldPos;
in vec3 vNormal;

out vec4 FragColor;

uniform vec3 cameraPos;
uniform sampler2D  envMap; // reflection cubemap OR reflection texture packed as cube
uniform vec3 sunDir;
uniform vec3 sunColor;
//uniform float waterIOR; // ~1.333 //unused for now
uniform vec3 deepColor; // underwater color
uniform float foamThreshold; // control foam

float fresnelSchlick(float cosTheta, float f0) {
    return f0 + (1.0 - f0) * pow(1.0 - cosTheta, 5.0);
}

void main() {
    vec3 N = normalize(vNormal);
    vec3 V = normalize(cameraPos - vWorldPos);

    // Fresnel (Schlick)
    float cosTheta = max(dot(N, V), 0.0);
    float F0 = 0.02; // base reflectivity for water
    float F = fresnelSchlick(cosTheta, F0);

    // Reflection vector
    vec3 R = reflect(-V, N);
    vec2 uv;
    uv.x = atan(R.z, R.x) / (2.0 * 3.14159265) + 0.5;
    uv.y = asin(clamp(R.y, -1.0, 1.0)) / 3.14159265 + 0.5;
    uv = clamp(uv, 0.0, 1.0);
    vec3 reflColor = texture(envMap, uv).rgb;

    // Simple specular (sun)
    vec3 L = normalize(-sunDir);
    float spec = pow(max(dot(reflect(-L, N), V), 0.0), 64.0);
    vec3 specColor = sunColor * spec;

    // Depth-based absorption (approx using world Y)
    float depthFactor = clamp((vWorldPos.y + 10.0) / 20.0, 0.0, 1.0); // tune per scene
    vec3 refractedColor = mix(deepColor, reflColor * 0.8, depthFactor);

    // foam (simple view-dependent)
    float foam = smoothstep(foamThreshold, foamThreshold + 0.05, 1.0 - N.y); // foam on steep slopes

    vec3 color = mix(refractedColor, reflColor, F);
    color = mix(color, vec3(0.5,0.7,1), foam * 0.5); // add foam white
    color += specColor;

    FragColor = vec4(color, 1.0);
}
