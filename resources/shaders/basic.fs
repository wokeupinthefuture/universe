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
}