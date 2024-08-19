#version 460 core
out vec4 FragColor;

in vec3 Normal;  
in vec3 FragPos; 
in vec2 TexCoords;
// texture samplers
uniform vec3 lightPos; 
uniform vec3 viewPos; 
uniform vec3 lightColor;
uniform vec3 objectColor;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D Normal1;
uniform bool hasTexture;

void main()
{
        FragColor = vec4(objectColor, 1.0);
}
