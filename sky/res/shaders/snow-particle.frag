#version 460
out vec4 FragColor;

in vec2 TexCoords;

uniform vec4 u_color;
uniform sampler2D snowflakeTexture;

void main()
{       
    FragColor = texture2D(snowflakeTexture, TexCoords).rgba;
}