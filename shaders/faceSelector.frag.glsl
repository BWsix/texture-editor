#version 430 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform vec3 lightPos;
vec3 lightColor = vec3(1);
uniform vec3 viewPos;

layout(location=0) out vec4 FragColor;
layout(location=1) out uvec4 id;

void main() {
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 normal = normalize(Normal);

    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * lightColor;

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = 0.5 * diff * lightColor;

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float specularStrength = 0.5;
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;  

    vec3 objectColor = vec3(gl_PrimitiveID % 10 * 0.04 + 0.6);
    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);

    id = uvec4(gl_PrimitiveID + 1, 0, 0, 0);
}
