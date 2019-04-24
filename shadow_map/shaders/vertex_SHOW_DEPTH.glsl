#version 330 core

layout (location = 0) in vec3 fragPos;
layout (location = 1) in vec2 texCoordsIn;

out vec2 texCoords;

void main()
{
    texCoords = texCoordsIn;
    gl_Position = vec4(fragPos, 1.0);
}
