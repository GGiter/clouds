#version 460
out vec4 FragColor;

in vec2 TexCoords;

uniform vec4 u_color;

void main()
{       
    FragColor = vec4(1,1,1,1);
    FragColor.a *= u_color.a;
}