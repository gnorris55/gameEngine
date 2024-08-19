#version 460 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;
layout (location = 3) out vec3 gLightNormal;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform vec3 objectColor = vec3(1.0, 1.0, 1.0);
uniform bool hasTexture;

void main()
{    
    // store the fragment position vector in the first gbuffer texture
    gPosition = FragPos;
    // also store the per-fragment normals into the gbuffer
    gNormal = normalize(Normal);
    // and the diffuse per-fragment color
    if (hasTexture) {
        gAlbedoSpec.rgb = texture(texture_diffuse1, TexCoords).rgb*objectColor;
        gAlbedoSpec.a = texture(texture_specular1, TexCoords).r;
        gLightNormal = texture(texture_normal1, TexCoords).rgb;
    } else {
        gAlbedoSpec.rgb = objectColor;
        gAlbedoSpec.a = 0.1;
        //gLightNormal = vec3(0, 0, 2);
    }
}
