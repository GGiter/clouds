#version 460
in vec2 v_TexCoord;
layout(location = 0) out vec4 FragColor;

uniform sampler2D Source; // The input texture

void main()
{
    // Sample the texture at the given texture coordinate
    vec4 texColor = texture(Source, v_TexCoord);

    // Output the sampled texture color
    FragColor = texColor;
}