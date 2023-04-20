#version 330 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;

    vec3 specular;
    vec3 diffuse;
    vec3 ambient;

    float constant;
    float linear;
    float quadratic;

    vec3 lightColor;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    vec3 color;
};

struct Material {
    sampler2D diffuse;
    sampler2D specular;

    float shininess;
};

#define NR_POINT_LIGHTS 2

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotlight;

uniform Material material;
uniform vec3 viewPosition;
uniform samplerCube skybox;


vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(viewPosition - FragPos);
    vec3 result = CalcDirLight(dirLight, normal, viewDir);
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], normal, FragPos, viewDir);
    result += CalcSpotLight(spotlight, normal, FragPos, viewDir);
    FragColor = vec4(result, 1.0);
}

// calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    return (ambient + diffuse + specular);
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
        vec3 lightDir = normalize(light.position - fragPos);
         // diffuse shading
         float diff = max(dot(normal, lightDir), 0.0);
         // specular shading
         vec3 halfwayDir = normalize(lightDir +  viewDir);
         float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
         // attenuation
         float distance = length(light.position - fragPos);
         float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
         // combine results
         vec3 ambient = (0.3 * light.lightColor) * light.ambient * vec3(texture(material.diffuse, TexCoords));
         vec3 diffuse =  (0.3 * light.lightColor) * light.lightColor * light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
         vec3 specular =  (0.3 * light.lightColor) *light.lightColor * light.specular * spec * vec3(texture(material.specular, TexCoords));
         ambient *= attenuation;
         diffuse *= attenuation;
         specular *= attenuation;
         return (ambient + diffuse + specular);
}
// calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results
    vec3 ambient = light.color * light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse = light.color * light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.color * light.specular * spec * vec3(texture(material.specular, TexCoords));
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}

