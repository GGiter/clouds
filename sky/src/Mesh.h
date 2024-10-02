#pragma once

#include <glad/glad.h> 
#include <string>
#include <vector>
#include "Vertex.h"
#include <memory>
#include "AABBCollision.h"

class Texture2D;
class Shader;

class Mesh {
public:
    // mesh Data
    std::vector<Vertex>       vertices;
    std::vector<unsigned int> indices;
    std::vector<std::unique_ptr<Texture2D>>  m_textures;
    unsigned int VAO;
    AABBCollision m_AABBCollision;
    // constructor
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<std::unique_ptr<Texture2D>> textures);
    virtual ~Mesh();
    void InitCollision(const glm::vec3& position, const::glm::vec3 scale);
    void SetPosition(const glm::vec3& position);
    // render the mesh
    void Draw(Shader &shader);
    void Uninitialize();

private:
    // render data 
    unsigned int VBO, EBO;

    // initializes all the buffer objects/arrays
    void setupMesh();
};