#version 460
layout(location = 0) in vec3 Position;

out vec3 v_TexCoord;

uniform mat4 projection;
uniform mat4 view;

void main() {
    v_TexCoord = Position;
    gl_Position = projection * view * vec4(Position, 1.0);
}