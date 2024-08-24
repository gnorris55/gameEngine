#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{    
    // FragColor = texture(skybox, TexCoords);
    float depthColor = texture(skybox, TexCoords).r;
    FragColor = vec4(depthColor, depthColor, depthColor, 1.0); 
}
