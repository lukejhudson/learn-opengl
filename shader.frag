#version 330 core

//struct Material {
//    vec3 ambient;
//    vec3 diffuse;
//    vec3 specular;
//    float shininess;
//};

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

//struct Light {
//    // Would have vec3 direction instead of position for a directional light
//    // (all rays parallel since light source infinitely far away, e.g. light from sun),
//    // then adjust the lightDir calculation to just be normalize(-light.direction);
//    vec3 position;
//    // Info for spotlights:
//    vec3 direction;
//    float cutOff; // Maximum angle of inner circle of light emitted
//    float outerCutOff; // Maximum angle of outer circle --> Blur edges of flashlight

//    vec3 ambient;
//    vec3 diffuse;
//    vec3 specular;

//    // Attenuation values - Decrease the light's intensity with distance
//    // (for point/spot lights, not directional lights)
//    float constant;
//    float linear;
//    float quadratic;
//};

// Directional light - All rays parallel coming from infinitely far light source, e.g. sun
// No attenuation
struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

// Point light - A light source in world space, shining in all directions; affected by attenuation
struct PointLight {
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

// Spot light - A flashlight shining a cone of light; affected by attenuation
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
};

#define NR_POINT_LIGHTS 4

in vec2 TexCoords; // Texture coordinates
in vec3 Normal;
in vec3 FragPos;

out vec4 FragColor;

//uniform vec4 ourColor; // We set this variable in the OpenGL code
uniform sampler2D texture1; // Actual texture image
uniform sampler2D texture2;

uniform vec3 viewPos; // Camera position
uniform Material material;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;

// Function prototypes
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main() {
    // Properties
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // == =====================================================
    // Our lighting is set up in 3 phases: directional, point lights and an optional flashlight
    // For each phase, a calculate function is defined that calculates the corresponding color
    // per lamp. In the main() function we take all the calculated colors and sum them up for
    // this fragment's final color.
    // == =====================================================

    // Directional light
    vec3 result = CalcDirLight(dirLight, norm, viewDir);
    // Point lights
    for (int i = 0; i < NR_POINT_LIGHTS; i++) {
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
    }
    // Spot light
    result += CalcSpotLight(spotLight, norm, FragPos, viewDir);

    FragColor = vec4(result, 1.0);

}

// Calculates the colour when using a directional light
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir) {
    // Ambient lighting
    vec3 ambient = light.ambient * vec3(texture2D(material.diffuse, TexCoords));

    // Diffuse lighting
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * vec3(texture2D(material.diffuse, TexCoords));

    // Specular lighting
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * vec3(texture2D(material.specular, TexCoords)));

    return ambient + diffuse + specular;
}

// Calculates the colour when using a point light
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    // Ambient lighting
    vec3 ambient = light.ambient * vec3(texture2D(material.diffuse, TexCoords));

    // Diffuse lighting
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * vec3(texture2D(material.diffuse, TexCoords));

    // Specular lighting
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * vec3(texture2D(material.specular, TexCoords)));

    // Attenuation
    float distance    = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance +
                        light.quadratic * (distance * distance));

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return ambient + diffuse + specular;
}

// Calculates the colour when using a spot light
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    // Ambient lighting
    vec3 ambient = light.ambient * vec3(texture2D(material.diffuse, TexCoords));

    // Diffuse lighting
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * vec3(texture2D(material.diffuse, TexCoords));

    // Specular lighting
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * vec3(texture2D(material.specular, TexCoords)));

    // Spotlight (soft edges) - Don't need if statement since the intensity is 0 for fragments outside of spotlight --> Just ambient colour
    float theta = dot(lightDir, normalize(-light.direction)); // Produces cosine of the angle between the two vectors
    float epsilon = (light.cutOff - light.outerCutOff);
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    // Attenuation
    float distance    = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance +
                        light.quadratic * (distance * distance));

    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    return ambient + diffuse + specular;
}




























