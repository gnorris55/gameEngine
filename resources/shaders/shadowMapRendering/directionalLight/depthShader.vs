#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main()
{
    vec4 fragPos = vec4(aPos, 1.0)*model;
    gl_Position = lightSpaceMatrix * fragPos;
}
