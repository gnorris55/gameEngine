#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;


out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;
out vec3 Tangent;
out vec3 Bitangent;

uniform mat4 model;
uniform mat4 projView;
uniform float texScalar = 1.0f;

void main()
{
    vec4 worldPos = vec4(aPos, 1.0)*model;
    FragPos = worldPos.xyz; 
    TexCoords = aTexCoords * texScalar;
    
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    Normal = normalize(aNormal * normalMatrix);
    Tangent = normalize(aTangent * normalMatrix);
    Bitangent = normalize(aBitangent * normalMatrix);
    

    gl_Position = projView * worldPos;
}