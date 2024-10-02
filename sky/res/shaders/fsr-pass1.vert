#version 460
#pragma name FSR_RCAS

// FSR - [RCAS] ROBUST CONTRAST ADAPTIVE SHARPENING

layout(location = 0) in vec4 Position;
layout(location = 1) in vec2 TexCoord;
out vec2 v_TexCoord;

void main() {
    gl_Position = Position;
    v_TexCoord = TexCoord;
}

