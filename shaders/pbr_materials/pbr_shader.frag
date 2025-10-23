#version 330 core

#define MAX_POINT_LIGHTS 64

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    float constant;
    float linear;
    float quadratic;
};

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

out vec4 FragColor;

uniform vec3 viewPos;

// PBR material
uniform vec3 materialAlbedo;
uniform float metallic;
uniform float roughness;
uniform float ao;

uniform sampler2D albedoMap;
uniform bool useAlbedoMap;

uniform bool ignoreLighting;

uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform int numPointLights;

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a      = roughness * roughness;
    float a2     = a * a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / (3.14159265 * denom * denom);
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

void main()
{
    vec3 albedo = materialAlbedo;
    if (useAlbedoMap){
        vec4 textureColor = texture(albedoMap, TexCoords);
        if(textureColor.a < 0.05){
            discard;
        }
        albedo = pow(textureColor.rgb, vec3(2.2)); // sRGB to linear
    }

if(!ignoreLighting){
    vec3 N = normalize(Normal);
    vec3 V = normalize(viewPos - FragPos);

    // Calculate reflectance at normal
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);

    for (int i = 0; i < numPointLights; ++i)
    {
        PointLight light = pointLights[i];
        vec3 L = normalize(light.position - FragPos);
        vec3 H = normalize(V + L);
        float distance = length(light.position - FragPos);
        float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);
        vec3 radiance = light.color * light.intensity * attenuation;

        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);
        vec3 F    = FresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
        vec3 specular     = numerator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic; // metals have no diffuse

        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / 3.14159265 + specular) * radiance * NdotL;
    }

    // Ambient lighting
    vec3 ambient = vec3(0.1) * albedo * ao;

    vec3 color = ambient + Lo;

    // Tone mapping and gamma correction
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2)); // linear to gamma

    FragColor = vec4(color, 1.0);
    }else{
        FragColor = vec4(albedo, 1.0);
    }
}