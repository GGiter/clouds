#include "Mesh.h"
#include "Shader.h"
#include "Texture2D.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "TextureCounter.h"
#include "Util.h"
#include "GameWorld.h"  

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<std::shared_ptr<Texture2D>> textures)
    : m_textures(textures)
{
    this->vertices = vertices;
    this->indices = indices;

    // now that we have all the required data, set the vertex buffers and its attribute pointers.
    setupMesh();
}

Mesh::~Mesh()
{
}

void Mesh::Uninitialize()
{
    auto it = std::find(GameWorld::AABBs.begin(), GameWorld::AABBs.end(), &m_AABBCollision);
    if (it != GameWorld::AABBs.end()) {
        GameWorld::AABBs.erase(it);
    }
}

void Mesh::InitCollision(const glm::vec3& position, const::glm::vec3 scale)
{
   m_AABBCollision = AABBCollision(position, scale);
   GameWorld::AABBs.push_back(&m_AABBCollision);
}

void Mesh::SetPosition(const glm::vec3& position)
{
    m_AABBCollision.SetPosition(position);
}

// render the mesh
void Mesh::Draw(Shader &shader, const glm::vec3& sceneScale) 
{
    // bind appropriate textures
    for(unsigned int i = 0; i < m_textures.size(); i++)
    {
        m_textures[i]->Bind(TextureCounter::GetNextID());
        std::string name = m_textures[i]->GetName() + std::to_string(i + 1);
        shader.SetUniform1i(name, m_textures[i]->GetSlot());
    }
        
    // draw mesh
    GLCall(glBindVertexArray(VAO));
    GLCall(glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0));
    GLCall(glBindVertexArray(0));

    for(unsigned int i = 0; i < m_textures.size(); i++)
    {
         m_textures[i]->Unbind();
    }
}
void Mesh::setupMesh()
{
    // create buffers/arrays
    GLCall(glGenVertexArrays(1, &VAO));
    GLCall(glGenBuffers(1, &VBO));
    GLCall(glGenBuffers(1, &EBO));

    GLCall(glBindVertexArray(VAO));
    // load data into vertex buffers
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, VBO));
    // A great thing about structs is that their memory layout is sequential for all its items.
    // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
    // again translates to 3/2 floats which translates to a byte array.
    GLCall(glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW));  

    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO));
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW));

    // set the vertex attribute pointers
    // vertex Positions
    GLCall(glEnableVertexAttribArray(0));	
    GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0));
    // vertex normals
    GLCall(glEnableVertexAttribArray(1));	
    GLCall(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal)));
    // vertex texture coords
    GLCall(glEnableVertexAttribArray(2));	
    GLCall(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords)));
    // vertex tangent
    GLCall(glEnableVertexAttribArray(3));
    GLCall(glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent)));
    // vertex bitangent
    GLCall(glEnableVertexAttribArray(4));
    GLCall(glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent)));
    // vertex color
    GLCall(glEnableVertexAttribArray(5));
    GLCall(glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, VertexColors)));
    GLCall(glBindVertexArray(0));
}