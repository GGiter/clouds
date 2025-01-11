#version 460

// Uniforms
uniform mat4 u_MVPM;
uniform mat4 model;
uniform mat4 view;
uniform mat4 lightSpaceMatrix;
uniform float snowAccumulation;
uniform float shadowIntensity;

// Vertex attributes
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;
layout(location = 5) in vec4 aVertexColor;

// Outputs to the fragment shader
out vec2 vTexCoords;
out vec3 vModelSpacePosition;
out vec3 vNormal;
out vec4 vVertexColor;
out vec3 vFragPos;
out vec4 vFragPosLightSpace;

void main() {
    // Pass through texture coordinates and vertex color
    vTexCoords = aTexCoords;    
    vVertexColor = aVertexColor;

    // Compute transformed position in clip space
    vec4 worldPosition = model * vec4(aPos, 1.0);
    gl_Position = u_MVPM * worldPosition;

    // Compute world space and model space positions
    vModelSpacePosition = worldPosition.xyz;
    vFragPos = aPos;

    // Compute normal in model space and world space
    vNormal = aNormal;

    // Calculate fragment position in light space
    vFragPosLightSpace = lightSpaceMatrix * vec4(vec3(worldPosition), 1.0);
}
