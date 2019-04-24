#version 330 core

out vec4 fragColor;

in vec3 fragPos;
in vec3 fragNormal;
in vec2 texCoords;
in vec4 fragPosLight;

uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;
uniform vec3 lightPos;
uniform vec3 viewPos;

float shadorFunc(vec4 fragPosLight)
{
    vec3 projCoords = fragPosLight.xyz / fragPosLight.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    vec3 normal = normalize(fragNormal);
    vec3 frag_light = normalize(lightPos - fragPos);
    float EPS = max(0.05 * (1.0 - dot(normal, frag_light)), 0.005);
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float curPCFDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - EPS > curPCFDepth  ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    if (projCoords.z > 1.0) {
        shadow = 0.0;
    }
    return shadow;
}

void main()
{
    vec3 color = texture(diffuseTexture, texCoords).rgb;
    vec3 normal = normalize(fragNormal);
    vec3 ambient = 0.25 * color;

    vec3 frag_light = normalize(lightPos - fragPos);
    vec3 diffuse = vec3(max(dot(frag_light, normal), 0.0));

    vec3 eye_frag = normalize(viewPos - fragPos);
    vec3 halfwayDir = normalize(eye_frag + frag_light);
    vec3 specular = vec3(pow(max(dot(normal, halfwayDir), 0.0), 512.0));

    float shadow = shadorFunc(fragPosLight);
    float lightIntensity = 8.0;
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular) * lightIntensity / distance(lightPos, fragPos) / distance(lightPos, fragPos)) * color;

    fragColor = vec4(lighting, 1.0);
}
