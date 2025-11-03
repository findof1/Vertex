#version 330 core
in vec3 vWorldPos;
in vec3 vNormal;
in vec4 vClipPos;
in vec2 vTexPos;

out vec4 FragColor;

uniform vec3 cameraPos;
uniform sampler2D reflectionTex;
uniform sampler2D refractionTex;
uniform sampler2D dudvMap;
uniform sampler2D normalMap;

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
    vec2 reflectTextCoords = vec2(uv.x, 1-uv.y);
	vec2 refractTexCoords = uv;

    float moveFactor = mod(0.05 * time, 1.0);
    vec2 distortedTexCoords = texture(dudvMap, vec2(vTexPos.x + moveFactor, vTexPos.y)).rg*0.1;
	distortedTexCoords = vTexPos + vec2(distortedTexCoords.x, distortedTexCoords.y+moveFactor);
	vec2 totalDistortion = (texture(dudvMap, distortedTexCoords).rg * 2.0 - 1.0) * 0.015;

	reflectTextCoords += totalDistortion;
	reflectTextCoords.x = clamp(reflectTextCoords.x, 0.001,0.999);
	reflectTextCoords.y = clamp(reflectTextCoords.y, 0.001, 0.999);

	refractTexCoords += totalDistortion;
	refractTexCoords = clamp(refractTexCoords, 0.001, 0.999);

    vec3 reflColor = texture(reflectionTex, reflectTextCoords).rgb;
    vec3 refrColor = texture(refractionTex, refractTexCoords).rgb;

    vec4 normalMapColor = texture(normalMap, distortedTexCoords);
	vec3 mapNormal = vec3(normalMapColor.r * 2.0 - 1.0, normalMapColor.b, normalMapColor.g * 2.0 - 1.0);
    float mapNormalFactor = 0.5;
     N = normalize(N + mapNormal * mapNormalFactor);

    // Simple specular (sun)
    vec3 L = normalize(-sunDir);
    float specularFactor = 4;
    float specularIntensity = 1.5;
    float spec = pow(max(dot(reflect(-L, N), V), 0.0), specularFactor);
    vec3 specColor = sunColor * spec * specularIntensity;

    float depthFactor = clamp((vWorldPos.y + 5) * 0.1 + 0.5, 0.0, 1.0); 
    vec3 bodyColor = mix(deepColor, refrColor, depthFactor);

    vec3 color = mix(bodyColor, reflColor, F);

    color += specColor * F;

    float foam = smoothstep(foamThreshold, foamThreshold + 0.05, 1.0 - N.y); // foam on steep slopes
    
    color = mix(color, vec3(1,1,1), foam); 

    FragColor = vec4(color, 1);
}
