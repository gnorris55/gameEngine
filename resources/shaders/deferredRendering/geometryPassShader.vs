#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;

uniform mat4 model;
uniform mat4 projView;
uniform float texScalar = 1.0f;

void main()
{
    vec4 worldPos = vec4(aPos, 1.0)*model;
    FragPos = worldPos.xyz; 
    TexCoords = aTexCoords * texScalar;
    
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    Normal = aNormal * normalMatrix;

    gl_Position = projView * worldPos;
}
