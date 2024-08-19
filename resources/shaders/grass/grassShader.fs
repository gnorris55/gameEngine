#version 450 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec3 Normal;
in vec3 FragPos;

uniform vec3 objectColor = vec3(1.0, 1.0, 1.0);
uniform bool hasTexture;
uniform vec3 viewPos = vec3(1, 1, 1);
uniform vec3 viewNormal = vec3(1, 1, 1);

bool normals_facing_eachother() {

    vec3 delta = FragPos - viewPos;
    //float dp0 = dot(delta, viewNormal);
    float dp1 = dot(delta, Normal);
    if (dp1 < 0)
        return true;
        
    return false;


}

void main()
{    
    // store the fragment position vector in the first gbuffer texture
    gPosition = FragPos;
    
    // also store the per-fragment normals into the gbuffer
    // and the diffuse per-fragment color
    if(normals_facing_eachother()) { 
        gNormal = normalize(Normal);
        //gAlbedoSpec.rgb = vec3(1, 1, 1);
    }
    else {
        gNormal = normalize(vec3(-Normal.x, Normal.y, -Normal.z));
        //gAlbedoSpec.rgb = vec3(1, 0, 0);
    }
    gAlbedoSpec.rgb = objectColor;
    gAlbedoSpec.a = 0.1;
}