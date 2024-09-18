#version 460 core

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shine;
};
struct Light {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

in vec2 TexCoords;
in vec3 Normal;  
in vec3 FragPos;  

uniform Material material;
uniform Light light;
uniform vec3 viewPos;

out vec4 FragColor;

void main() {
    vec3 lightDir = normalize(light.position - FragPos);
    // cutoff
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // attenuation
    float light_constant = 1.2;
    float distance    = length(light.position - FragPos);
    float attenuation = light_constant * 1.0 / (light.constant + light.linear * distance +  light.quadratic * (distance * distance));
    // ambient
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    // diffuse 
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, lightDir), 0);
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));  
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shine);
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));          
    diffuse *=  intensity;
    specular *=  intensity;
    FragColor = vec4(ambient + diffuse + specular, 1.0); 
}
