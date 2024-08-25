#version 460 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D gLightNormal;
uniform sampler2D gTangent;
uniform sampler2D gBitangent;
uniform sampler2D directionalShadowMap;

//todo: tempary
uniform float far_plane;

struct Light {
    vec3 Position;
    vec3 Color;
    
    float Linear;
    float Quadratic;
    float Radius;
};

const int NR_LIGHTS = 32;
uniform Light lights[NR_LIGHTS];
uniform samplerCube shadowMap1;
uniform samplerCube shadowMap2;
uniform samplerCube shadowMap3;
uniform samplerCube shadowMap4;

uniform vec3 viewPos;
uniform bool hasTexture = true;

// for directional
uniform Light sunLight;
uniform mat4 lightSpaceMatrix;



float shadow_calculation(vec3 FragPos, vec3 Normal) {
   
    // for shadow acne
    vec4 FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0); 
    vec3 lightDir = normalize(sunLight.Position - FragPos);
    float bias = max(0.05 * (1.0 - dot(Normal, lightDir)), 0.005);  


    vec3 projCoords = FragPosLightSpace.xyz / FragPosLightSpace.w;

    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(directionalShadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float shadow = 0;

    vec2 texelSize = 1.0 / textureSize(directionalShadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(directionalShadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;

    if (projCoords.z > 1.0)
        shadow = 0;

    return shadow;
}

float point_shadow_calculation(vec3 FragPos, vec3 lightPos, int index) {
    
    vec3 fragToLight = FragPos - lightPos;
    float closestDepth;
    if (index == 0)
        closestDepth = texture(shadowMap1, fragToLight).r;
    else if (index == 1)
        closestDepth = texture(shadowMap2, fragToLight).r;
    else if (index == 2)
        closestDepth = texture(shadowMap3, fragToLight).r;
    else if (index == 3)
        closestDepth = texture(shadowMap4, fragToLight).r;
    closestDepth *= far_plane;
    
    float currentDepth = length(fragToLight);
    float bias = 0.05; // we use a much larger bias since depth is now in [near_plane, far_plane] range
    float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;

    return shadow;

}

vec3 calculate_blinn_phong(vec3 FragPos, vec3 Normal, vec3 Diffuse, float Specular) {
    

    vec3 normalMap = texture(gLightNormal, TexCoords).rgb;
    vec3 normal = vec3(0, 0, 0);

    if (length(normalMap) > 0) {
        vec3 T = texture(gTangent, TexCoords).rgb;
        vec3 B = texture(gBitangent, TexCoords).rgb;

        T = normalize(T-dot(T, Normal)*Normal);
        B = normalize(B-dot(B, Normal)*Normal);

        mat3 TBN = mat3(T, B, Normal);

        normal = texture(gLightNormal, TexCoords).rgb;
        normal = normal * 2.0 - 1.0;
        normal = normalize(TBN*normal);
    } else {
        normal = Normal;
    }

    float gamma = 2.2f;  
    vec3 lighting  = vec3(0, 0, 0);
    vec3 diffuse = vec3(0, 0, 0);
    vec3 specular = vec3(0, 0, 0);

    
     vec3 viewDir  = normalize(viewPos - FragPos);
    // lighting calculations
    for(int i = 0; i < NR_LIGHTS; ++i)
    {
        // calculate distance between light source and current fragment
        float distance = length(lights[i].Position - FragPos);
        if(distance < lights[i].Radius)
        {
                       
            float shadow = point_shadow_calculation(FragPos, lights[i].Position, i);
            // diffuse
            vec3 lightDir = normalize(lights[i].Position - FragPos);
            
            vec3 curr_diffuse = max(dot(normal, lightDir), 0.0) * Diffuse * lights[i].Color;
            // specular
            vec3 halfwayDir = normalize(lightDir + viewDir);  
            float spec = pow(max(dot(normal, halfwayDir), 0.0), 16.0);
            vec3 curr_specular = lights[i].Color * spec * Specular;
            // attenuation
            float attenuation = 1.0 / (1.0 + lights[i].Linear * distance + lights[i].Quadratic * distance * distance);
            curr_diffuse *= attenuation;
            curr_specular *= attenuation;

            specular += curr_specular*(1-shadow);
            diffuse += curr_diffuse*(1-shadow);
        }
    }


    float shadow = shadow_calculation(FragPos, Normal);

    vec3 lightDir = normalize(sunLight.Position - FragPos);
            
    vec3 curr_diffuse = max(dot(normal, lightDir), 0.0) * Diffuse * sunLight.Color;
           
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 16.0);
    vec3 curr_specular = sunLight.Color * spec * Specular;
    
    specular += curr_specular*(1-shadow);
    diffuse += curr_diffuse*(1-shadow);

    lighting += diffuse + specular;

    return lighting;
}


void main()
{            

    // retrieve data from gbuffer
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Diffuse = texture(gAlbedoSpec, TexCoords).rgb;
    float Specular = texture(gAlbedoSpec, TexCoords).a;
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    
    // calculate lighting   
    vec3 ambient = Diffuse * 0.1;
    vec3 normalMap = texture(gLightNormal, TexCoords).rgb;
    
    vec3 diffuse_specular =  calculate_blinn_phong(FragPos, Normal, Diffuse, Specular);
         
    // calculate shadows
    //float shadow = shadow_calculation(FragPos, Normal);
   
    vec3 result = ambient + (diffuse_specular);

    // for tone mapping
    float exposure = 0.5;
    vec3 mapped = vec3(1.0) - exp(-result * exposure);

    // for gamma correction
    result = pow(result, vec3(1.0/2.2));

    FragColor = vec4(result, 1.0);
}