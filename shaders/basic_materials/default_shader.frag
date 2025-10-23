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
uniform vec3 materialAlbedo;      
uniform sampler2D albedoMap;    
uniform bool useAlbedoMap;
uniform bool ignoreLighting;

uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform int numPointLights;

void main() {
    vec3 albedo = materialAlbedo;
    if (useAlbedoMap){
        vec4 textureColor = texture(albedoMap, TexCoords);
        if(textureColor.a < 0.05){
            discard;
        }
        albedo = textureColor.rbg;
    }

    if(!ignoreLighting){
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 lighting = vec3(0.0);

    // Loop over all point lights
    for (int i = 0; i < numPointLights; i++)
    {
        PointLight light = pointLights[i];

        // Direction from fragment to light
        vec3 lightDir = normalize(light.position - FragPos);

        // Diffuse component
        float diff = max(dot(norm, lightDir), 0.0);

        // Specular (Blinn-Phong)
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);

        // Attenuation
        float distance    = length(light.position - FragPos);
        float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

        // Combine
        vec3 diffuse  = diff * light.color * light.intensity;
        vec3 specular = spec * light.color * light.intensity * 0.5; // tweak spec strength
    lighting += (diffuse + specular) * attenuation;
        
    }

    lighting += vec3(clamp(lighting.x,0.1,1),clamp(lighting.y,0.1,1),clamp(lighting.z,0.1,1));
    vec3 finalColor = albedo * lighting;
    FragColor = vec4(finalColor, 1.0);
    }else{
        FragColor = vec4(albedo, 1.0);;
    }
}