#pragma once

#include "glm/glm.hpp"


struct VertexSimplified
{
	glm::vec3 position;
	glm::vec2 TexCoords;
};

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
	glm::vec4 VertexColors;
};