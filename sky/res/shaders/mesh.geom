#version 460 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 9) out;

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

void main()
{
    vec3 N[3];
    vec3 displacedPosition[3];
    
    for (int i = 0; i < 3; i++)
    {
        // Normalize the normal vector
        N[i] = normalize(vNormal[i]);
        
        // Calculate the slope
        float slope = acos(dot(N[i], vec3(0.0, -1.0, 0.0)));
        slope = clamp(slope / 3.14159, 0.0, 1.0);
        slope = pow(slope, 2.0);
        // Calculate displacement
        vec3 displacement = slope * N[i] * snowAccumulation * maxDisplacement;
        
        // Apply displacement to vertex position
        displacedPosition[i] = displacement;

        // Emit the original vertices
        gl_Position = gl_in[i].gl_Position;
        TexCoords = vTexCoords[i];
        position = vPosition[i];
        ModelSpacePosition = vModelSpacePosition[i];
        normal = N[i];
        ModelSpaceNormal = vModelSpaceNormal[i];
        vertexColor = vVertexColor[i];
        FragPos = vFragPos[i];
        TBN = vTBN[i];
        FragPosLightSpace = vFragPosLightSpace[i];
        EmitVertex();
    }
    EndPrimitive();

    for(float alpha = 0.1; alpha < 1.0; alpha+= 0.01)
    {
        for (int i = 0; i < 3; i++)
        {
            // Emit the original vertices with displacement
            gl_Position = gl_in[i].gl_Position + vec4(displacedPosition[i] * alpha, 0.0);
            TexCoords = vTexCoords[i];
            position = displacedPosition[i];
            ModelSpacePosition = vModelSpacePosition[i];
            normal = N[i];
            ModelSpaceNormal = vModelSpaceNormal[i];
            vertexColor = vVertexColor[i];
            FragPos = vFragPos[i]; //HACK
            TBN = vTBN[i];
            FragPosLightSpace = vFragPosLightSpace[i];
            EmitVertex();
        }
    }

   EndPrimitive();
    

}
