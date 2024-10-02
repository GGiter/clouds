#version 460
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform vec4 u_color;

void main()
{       
    FragColor = texture(texture_diffuse1,TexCoords);
    FragColor.a *= u_color.a;
}