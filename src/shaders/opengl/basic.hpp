#pragma once

namespace BasicShader
{

#if LOAD_SHADER_FROM_SOURCE == false

static constexpr auto vs = "resources/shaders/basic.vs";
static constexpr auto fs = "resources/shaders/basic.fs";

#else

static constexpr auto fs = R"(
#version 330

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

uniform vec3 color;

// Output fragment color
out vec4 finalColor;

void main()
{
    finalColor = vec4(color, 1.0);
})";

static constexpr auto vs = R"(
#version 330

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
out vec3 fragPosition;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;

// NOTE: Add your custom variables here

void main()
{
    // Send vertex attributes to fragment shader
    fragPosition = vec3(matModel*vec4(vertexPosition, 1.0));
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    fragNormal = normalize(vec3(matNormal*vec4(vertexNormal, 1.0)));

    // Calculate final vertex position
    gl_Position = mvp * vec4(vertexPosition, 1.0);
})";

#endif

}  // namespace BasicShader
