#version 330 core

layout (location = 0) in vec3 fragPosIn;
layout (location = 1) in vec3 texNormalIn;
layout (location = 2) in vec2 texCoordsIn;

out vec3 fragPos;
out vec3 fragNormal;
out vec2 texCoords;
out vec4 fragPosLight;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 lightVP;

void main()
{
    fragPos = vec3(model * vec4(fragPosIn, 1.0));
    fragNormal = transpose(inverse(mat3(model))) * texNormalIn;
    texCoords = texCoordsIn;
    fragPosLight = lightVP * vec4(fragPos, 1.0);
    gl_Position = projection * view * vec4(fragPos, 1.0);
}
