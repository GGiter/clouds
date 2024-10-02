#version 460

layout(location = 0) in vec3 Position;

uniform mat4 depthMVP;
uniform mat4 model;

void main() {
    gl_Position = depthMVP * model * vec4(Position, 1.0);
}