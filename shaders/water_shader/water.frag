#version 330 core
in vec3 vWorldPos;
in vec3 vNormal;

out vec4 FragColor;

uniform vec3 cameraPos;
uniform sampler2D  envMap; // reflection cubemap OR reflection texture packed as cube
uniform vec3 sunDir;
uniform vec3 sunColor;
uniform float time;
//uniform float waterIOR; // ~1.333 //unused for now
uniform vec3 deepColor; // underwater color
uniform float foamThreshold; // control foam

float fresnelSchlick(float cosTheta, float f0) {
    return f0 + (1.0 - f0) * pow(1.0 - cosTheta, 5.0);
}

float hash(vec2 p) {
    p = vec2(dot(p, vec2(127.1,311.7)),
             dot(p, vec2(269.5,183.3)));
    return fract(sin(p.x+p.y) * 43758.5453123);
}

// 2D smooth noise
float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);

    // Four corners
    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));

    // Smooth interpolation
    vec2 u = f*f*(3.0-2.0*f);

    return mix(a, b, u.x) +
           (c - a)* u.y * (1.0 - u.x) +
           (d - b) * u.x * u.y;
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
    float phi = atan(R.z, R.x);      // -pi .. pi
    float theta = asin(clamp(R.y, -1.0, 1.0)); // -pi/2 .. pi/2

    uv.x = (phi + 3.14159265) / (2.0 * 3.14159265); // 0..1
    uv.y = (theta + 3.14159265 / 2.0) / 3.14159265; // 0..1
    uv.x = fract(uv.x); // wrap horizontally
    uv.y = clamp(uv.y, 0.0, 1.0); // clamp vertically
    vec3 reflColor = max((texture(envMap, uv).rgb + vec3(0,0.05,0.1))*2, vec3(0,0.3,0.7));

    // Simple specular (sun)
    vec3 L = normalize(-sunDir);
    float spec = pow(max(dot(reflect(-L, N), V), 0.0), 64.0);
    vec3 specColor = sunColor * spec;

    // Depth-based absorption (approx using world Y)
    float depthFactor = clamp((vWorldPos.y + 17.0) / 20.0, 0.2, 1.0); // tune per scene
    vec3 refractedColor = mix(deepColor, reflColor * 0.8, depthFactor);

    float depthFactor2 = clamp((vWorldPos.y + 17.0) / 20.0, 0.2, 1.0); // tune per scene
    vec3 refractedColor2 = mix(deepColor * 2, reflColor * 0.8, depthFactor2);

    // foam (simple view-dependent)
    float foam = smoothstep(foamThreshold, foamThreshold + 0.05, 1.0 - N.y); // foam on steep slopes
    
    float patternScale = 0.5; // larger = more stretched
    float pattern = noise(vWorldPos.xz * patternScale);
    pattern = pattern * 0.5 + 0.5; // normalize 0..1

    vec3 color = mix(refractedColor, reflColor, F);
    color = mix(color, vec3(1,1,1), foam * 0.8); // add foam white
    color += specColor;
    vec3 color2 = mix(refractedColor2, reflColor, F);
    color = mix(color, color2, pattern);

    FragColor = vec4(color, 1);
}
