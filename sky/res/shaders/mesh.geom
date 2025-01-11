#version 460 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

in vec2 vTexCoords[];
in vec3 vPosition[];
in vec3 vModelSpacePosition[];
in vec3 vNormal[];
in vec3 vModelSpaceNormal[];
in vec4 vVertexColor[];
in vec3 vFragPos[];
in mat3 vTBN[];
in vec4 vFragPosLightSpace[];

out vec2 TexCoords;
out vec3 position;
out vec3 ModelSpacePosition;
out vec3 normal;
out vec3 ModelSpaceNormal;
out vec4 vertexColor;
out vec3 FragPos;
out mat3 TBN;
out vec4 FragPosLightSpace;

uniform float snowAccumulation;
uniform float maxDisplacement;

vec3 calculateDisplacement(int i, out vec3 normalizedNormal) {
    normalizedNormal = normalize(vNormal[i]);
    float slope = acos(dot(normalizedNormal, vec3(0.0, -1.0, 0.0)));
    slope = pow(clamp(slope / 3.14159, 0.0, 1.0), 2.0);
    return slope * normalizedNormal * snowAccumulation * maxDisplacement;
}

void emitVertexData(int i, vec3 displacement) {
    gl_Position = gl_in[i].gl_Position + vec4(displacement, 0.0);
    TexCoords = vTexCoords[i];
    position = vPosition[i] + displacement;
    ModelSpacePosition = vModelSpacePosition[i] + displacement;
    normal = normalize(vNormal[i]);
    ModelSpaceNormal = vModelSpaceNormal[i];
    vertexColor = vVertexColor[i];
    FragPos = vFragPos[i] + displacement;
    TBN = vTBN[i];
    FragPosLightSpace = vFragPosLightSpace[i];
    EmitVertex();
}

void main() {
    vec3 N[3];
    vec3 displacedPosition[3];

    // Calculate displacement for each vertex
    for (int i = 0; i < 3; i++) {
        displacedPosition[i] = calculateDisplacement(i, N[i]);
    }

    // Emit original vertices
    for (int i = 0; i < 3; i++) {
        emitVertexData(i, vec3(0.0));
    }
    EndPrimitive();

    // Emit displaced vertices
    for (int i = 0; i < 3; i++) {
        emitVertexData(i, displacedPosition[i]);
    }
    EndPrimitive();

    // Emit connecting triangles
    for (int i = 0; i < 3; i++) {
        int next = (i + 1) % 3;

        emitVertexData(i, vec3(0.0));             // Original vertex
        emitVertexData(i, displacedPosition[i]);  // Displaced current vertex
        emitVertexData(next, displacedPosition[next]); // Displaced next vertex
    }
    EndPrimitive();
}
