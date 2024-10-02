#version 460
uniform mat4 u_MVPM;
uniform mat4 model;
uniform mat4 view;

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 5) in vec4 aVertexColor;

out vec2 TexCoords;
out vec3 position;
out vec3 normal;
out vec4 vertexColor;

void main()
{
    TexCoords = aTexCoords;    
    gl_Position = u_MVPM * model * vec4(aPos, 1.0);
    
    position = (model * vec4(aPos, 1.0)).xyz;
    normal = normalize(mat3(transpose(inverse(model))) * aNormal);
    vertexColor = aVertexColor;
}