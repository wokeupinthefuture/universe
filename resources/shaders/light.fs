#version 330

in vec3 fragPos;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

uniform float ambientStrength;
uniform float diffuseStrength;
uniform float specularExp;

out vec4 finalColor;

void main()
{
    vec3 ambientColor = lightColor * ambientStrength;
    
    vec3 lightDir = normalize(lightPos - fragPos);
    float diffuseStrength = max(dot(lightDir, fragNormal), 0.0);
    vec3 diffuseColor = lightColor * diffuseStrength;

    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, fragNormal);

    float specularStrength = pow(max(dot(viewDir, reflectDir), 0.0), specularExp);
    vec3 specularColor = lightColor * specularStrength;

    vec3 result = (ambientColor + diffuseColor + specularColor) * fragColor.xyz;

    finalColor = vec4(result, 1.0);
}