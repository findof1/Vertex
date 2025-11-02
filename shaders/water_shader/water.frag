#version 330 core
in vec3 vWorldPos;
in vec3 vNormal;
in vec4 vClipPos;

out vec4 FragColor;

uniform vec3 cameraPos;
uniform sampler2D reflectionTex;
uniform sampler2D refractionTex;

uniform vec3 sunDir;
uniform vec3 sunColor;
uniform float time;
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

    vec3 ndc = vClipPos.xyz / vClipPos.w; 
    vec2 uv;
    uv.x = ndc.x * 0.5 + 0.5;
    uv.y = ndc.y * 0.5 + 0.5;
    vec3 reflColor = texture(reflectionTex, vec2(uv.x, 1-uv.y)).rgb;
    vec3 refrColor = texture(refractionTex, uv).rgb;

    // Simple specular (sun)
    vec3 L = normalize(-sunDir);
    float spec = pow(max(dot(reflect(-L, N), V), 0.0), 64.0);
    vec3 specColor = sunColor * spec;

    float depthFactor = clamp((vWorldPos.y + 5) * 0.1 + 0.5, 0.0, 1.0); 
    vec3 bodyColor = mix(deepColor, refrColor, depthFactor);

    vec3 color = mix(bodyColor, reflColor, F);

    color += specColor * F;

    float foam = smoothstep(foamThreshold, foamThreshold + 0.05, 1.0 - N.y); // foam on steep slopes
    
    color = mix(color, vec3(1,1,1), foam); 

    FragColor = vec4(color, 1);
}
