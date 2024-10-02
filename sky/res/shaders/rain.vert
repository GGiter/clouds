#version 460
uniform mat4 u_MVPM;
uniform mat4 model;

layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoords;
layout (location = 1) in vec3 aNormal;

out vec2 TexCoords;

void main()
{
    TexCoords = aTexCoords;    
    gl_Position = u_MVPM * model * vec4(aPos, 1.0);
}