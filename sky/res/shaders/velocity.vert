#version 460

uniform mat4 uModelViewProjectionMat;
uniform mat4 uPrevModelViewProjectionMat;

smooth out vec4 vPosition;
smooth out vec4 vPrevPosition;

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;

void main() 
{
    vPosition = uModelViewProjectionMat * vec4(position, 1.0);
    vPrevPosition = uPrevModelViewProjectionMat * vec4(position, 1.0);
    gl_Position = vPosition;
}