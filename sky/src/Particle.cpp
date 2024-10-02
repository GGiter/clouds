#include "Particle.h"
#include "Vertex.h"
#include "VertexBufferLayout.h"

Particle::Particle(const glm::vec3& InVelocity)
	: Position(0.0f), Velocity(0.0f), Color(1.0f), Life(5.0f), bIs2D(true)
{
    transform = glm::mat4(1.f);
    transform = glm::scale(transform, glm::vec3(0.1f, 0.1f, 0.1f));
    Velocity = InVelocity;
}

void Particle::Initialize()
{
    std::vector<VertexSimplified> positions = {
             {glm::vec3(-1.0, -1.0, 1.0), glm::vec2(0.0f, 0.0f)},  {glm::vec3(1.0,  -1.0, 1.0), glm::vec2(1.0f, 0.0f)},  {glm::vec3(1.0,  1.0,  1.0), glm::vec2(1.0f, 1.0f)},
             {glm::vec3(-1.0, -1.0, 1.0), glm::vec2(0.0f, 0.0f)},  {glm::vec3(1.0,  1.0,  1.0), glm::vec2(1.0f, 1.0f)},  {glm::vec3(-1.0, 1.0,  1.0), glm::vec2(0.0f, 1.0f)},

             {glm::vec3( 1.0,  -1.0, 1.0), glm::vec2(0.0f, 0.0f)},  {glm::vec3(1.0,  -1.0, -1.0), glm::vec2(1.0f, 0.0f)}, {glm::vec3(1.0,  1.0,  -1.0), glm::vec2(1.0f, 1.0f)},
             {glm::vec3(1.0,  -1.0, 1.0), glm::vec2(0.0f, 0.0f)},  {glm::vec3(1.0,  1.0,  -1.0), glm::vec2(1.0f, 1.0f)}, {glm::vec3(1.0,  1.0,  1.0), glm::vec2(0.0f, 1.0f)},

             {glm::vec3(1.0,  -1.0, -1.0), glm::vec2(0.0f, 0.0f)}, {glm::vec3(-1.0, -1.0, -1.0), glm::vec2(1.0f, 0.0f)}, {glm::vec3(-1.0, 1.0,  -1.0), glm::vec2(1.0f, 1.0f)},
             {glm::vec3(1.0,  -1.0, -1.0), glm::vec2(0.0f, 0.0f)}, {glm::vec3(-1.0, 1.0,  -1.0), glm::vec2(1.0f, 1.0f)}, {glm::vec3(1.0,  1.0,  -1.0), glm::vec2(0.0f, 1.0f)},

             {glm::vec3(-1.0, -1.0, -1.0), glm::vec2(0.0f, 0.0f)}, {glm::vec3(-1.0, -1.0, 1.0), glm::vec2(1.0f, 0.0f)},  {glm::vec3(-1.0, 1.0,  1.0), glm::vec2(1.0f, 1.0f)},
             {glm::vec3(-1.0, -1.0, -1.0), glm::vec2(0.0f, 0.0f)}, {glm::vec3(-1.0, 1.0,  1.0), glm::vec2(1.0f, 1.0f)},  {glm::vec3(-1.0, 1.0,  -1.0), glm::vec2(0.0f, 1.0f)},

             {glm::vec3(-1.0, 1.0,  1.0), glm::vec2(0.0f, 0.0f)}, {glm::vec3( 1.0,  1.0,  1.0), glm::vec2(1.0f, 0.0f)},  {glm::vec3(1.0,  1.0,  -1.0), glm::vec2(1.0f, 1.0f)},
             {glm::vec3(-1.0, 1.0,  1.0), glm::vec2(0.0f, 0.0f)},  {glm::vec3(1.0,  1.0,  -1.0), glm::vec2(1.0f, 1.0f)}, {glm::vec3(-1.0, 1.0,  -1.0), glm::vec2(0.0f, 1.0f)},

             {glm::vec3(-1.0, -1.0, 1.0), glm::vec2(0.0f, 0.0f)},  {glm::vec3(-1.0, -1.0, -1.0), glm::vec2(1.0f, 0.0f)}, {glm::vec3(1.0,  -1.0, -1.0), glm::vec2(1.0f, 1.0f)},
             {glm::vec3(-1.0, -1.0, 1.0), glm::vec2(0.0f, 0.0f)}, {glm::vec3( 1.0,  -1.0, -1.0), glm::vec2(1.0f, 1.0f)}, {glm::vec3(1.0,  -1.0, 1.0), glm::vec2(0.0f, 1.0f)}};

    std::vector<unsigned int> indices;
    for (unsigned int i = 0; i < positions.size(); ++i) {
        indices.push_back(i);
	}

    if(bIs2D)
    {
       positions = {
      {glm::vec3(-1.f, -1.f, 0.0f), glm::vec2(0.0f, 0.0f)},
      {glm::vec3(1.f, -1.f, 0.0f), glm::vec2(1.0f, 0.0f)},
	  {glm::vec3(1.f, 1.f, 0.0f), glm::vec2(1.0f, 1.0f)},
      {glm::vec3(-1.f, 1.f, 0.0f), glm::vec2(0.0f, 1.0f)}
	    };
      indices = {0, 1, 2, 2, 3, 0};
    }


    vaParticle =  std::make_unique<VertexArray>();
	vbParticle =  std::make_unique<VertexBuffer>((unsigned int*)positions.data(), (unsigned int)positions.size() * 5 * sizeof(float));

	VertexBufferLayout layout;
	layout.Push<float>(3);
	layout.Push<float>(2);
	vaParticle->AddBuffer(*vbParticle, layout);

    ibParticle = std::make_unique<IndexBuffer>((unsigned int*)indices.data(), (unsigned int)indices.size());
}

void Particle::Draw(Renderer& renderer, const glm::mat4& MVP,Shader* particleShader)
{
    if (Life > 0.0f)
    {
        particleShader->Bind();
        transform = glm::mat4(1.f);
        transform = glm::scale(transform, GetScale());
        transform = glm::translate(transform, glm::vec3(Position.x, Position.z, Position.y));
        particleShader->SetUniformMat4f("u_MVPM", MVP);
        particleShader->SetUniformMat4f("transform", transform);
        particleShader->SetUniform4f("u_color", Color.r, Color.g, Color.b, Color.a);
        renderer.Draw(*vaParticle, *ibParticle, *particleShader);
        particleShader->Unbind();
    } 
}

glm::vec3 Particle::GetScale() const
{
    return glm::vec3(0.01f, 0.01f, 0.01f);
}
