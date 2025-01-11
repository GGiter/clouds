#version 460
#extension GL_NV_shadow_samplers_cube : enable
in vec3 v_TexCoord;
layout(location = 0) out vec4 FragColor;

uniform samplerCube Source; // The input texture

void main()
{
    // Sample the texture at the given texture coordinate
    vec4 texColor = texture(Source, v_TexCoord);

    // Output the sampled texture color
    FragColor = texColor;
}