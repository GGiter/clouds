#version 460

layout(location = 0) out vec4 fragmentdepth;

void main() {
    fragmentdepth = vec4(gl_FragCoord.z);
    fragmentdepth.a = 1.0;
}