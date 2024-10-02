#version 460
uniform mat4 u_MVPM;
uniform mat4 model;
uniform mat4 view;
uniform mat4 lightSpaceMatrix;

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
layout (location = 5) in vec4 aVertexColor;

out vec2 vTexCoords;
out vec3 vPosition;
out vec3 vModelSpacePosition;
out vec3 vNormal;
out vec3 vModelSpaceNormal;
out vec4 vVertexColor;
out vec3 vFragPos;
out mat3 vTBN;
out vec4 vFragPosLightSpace;

uniform float snowAccumulation;

//TO-DO vertex displacement
void main()
{
    vTexCoords = aTexCoords;    

    //normal = mat3(transpose(inverse(view * model))) * aNormal;
    vVertexColor = aVertexColor;
    gl_Position = u_MVPM * model * (vec4(aPos, 1.0));
    
    vPosition = (view * model * (vec4(aPos, 1.0))).xyz;
    vModelSpacePosition = (model * (vec4(aPos, 1.0))).xyz;
    vFragPos = aPos;
    vNormal = aNormal;
    vModelSpaceNormal = normalize(mat3(transpose(inverse(model))) * aNormal);
    
    vec3 T = normalize(vec3(model * vec4(aTangent,   0.0)));
    vec3 B = normalize(vec3(model * vec4(aBitangent, 0.0)));
    vec3 N = normalize(vec3(model * vec4(aNormal,    0.0)));
    vTBN = mat3(T, B, N);
    vFragPosLightSpace = lightSpaceMatrix * vec4(vec3(model* vec4(aPos, 1.0)), 1.0);
}