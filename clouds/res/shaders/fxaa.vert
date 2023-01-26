#version 460
layout(location = 0) in vec4 Position;
layout(location = 1) in vec2 TexCoord;
out vec2 v_TexCoord;

void main() {
    gl_Position = Position;
    v_TexCoord = TexCoord;
}