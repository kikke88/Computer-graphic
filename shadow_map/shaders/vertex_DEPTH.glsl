#version 330 core

layout (location = 0) in vec3 fragPos;

uniform mat4 lightVP;
uniform mat4 model;

void main()
{
    gl_Position = lightVP * model * vec4(fragPos, 1.0);
}
