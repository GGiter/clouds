#version 460
in vec3 v_TexCoord;
layout(location = 0) out vec4 FragColor;

uniform samplerCube Source;  // The first texture
uniform samplerCube Target;  // The second texture
uniform float mixFactor;   // The blend factor (0.0 = all Source, 1.0 = all Target)

void main()
{
    // Sample both textures at the given texture coordinate
    vec4 sourceColor = texture(Source, v_TexCoord);
    vec4 targetColor = texture(Target, v_TexCoord);

    // Linearly interpolate between the source and target textures
    vec4 mixedColor = mix(sourceColor, targetColor, mixFactor);

    // Output the interpolated texture color
    FragColor = targetColor;
}