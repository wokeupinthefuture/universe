#pragma once

namespace LightReceiverShader
{

#if LOAD_SHADER_FROM_SOURCE == false

static constexpr auto vs = "resources/shaders/light.vs";
static constexpr auto fs = "resources/shaders/light.fs";

#else

static constexpr auto fs = R"(
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

    finalColor = vec4(1.0, 0.1, 0.7, 1.0);
})";

static constexpr auto vs = R"(#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

// Input uniform values
uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;

// Output vertex attributes (to fragment shader)
out vec3 fragPos;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;

// NOTE: Add your custom variables here

void main()
{
    fragPos = vec3(matModel*vec4(vertexPosition, 1.0));
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    fragNormal = normalize(vec3(matNormal*vec4(vertexNormal, 1.0)));

    gl_Position = mvp*vec4(vertexPosition, 1.0);
})";

#endif

}  // namespace LightReceiverShader
