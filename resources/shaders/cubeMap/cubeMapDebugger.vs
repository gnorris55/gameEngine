#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    vec3 FragPos = vec3(vec4(aPos, 1) * model); 
    TexCoords = aPos;
    vec4 pos = projection * view * vec4(FragPos, 1);
    gl_Position = pos;
}  
