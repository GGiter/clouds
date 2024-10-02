#include "Framebuffer.h"

Framebuffer::Framebuffer(int windowWidth, int windowHeight) 
{
  m_texture2D = std::make_unique<Texture2D>(windowWidth, windowHeight);
  GLCall(glGenFramebuffers(1, &m_frameBufferID));
  GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferID));

  GLCall(glBindTexture(GL_TEXTURE_2D, m_texture2D->GetID()));
  GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D,
                         m_texture2D->GetID(), 0));
  GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

Framebuffer::~Framebuffer() 
{
}

void Framebuffer::Bind(int textureSlot) 
{
  GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferID));
  m_texture2D->Bind(textureSlot);
}

void Framebuffer::Unbind()
{ 
  GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
  m_texture2D->Unbind();
}

void Framebuffer::Draw() 
{
  GLenum bufferlist[] = {GL_COLOR_ATTACHMENT0};
  GLCall(glDrawBuffers(1, bufferlist));
}

DepthFramebuffer::DepthFramebuffer(int windowWidth, int windowHeight)
    : Framebuffer(windowWidth, windowHeight)
{
      m_texture2D = std::make_unique<DepthTexture2D>(windowWidth, windowHeight);
      GLCall(glGenFramebuffers(1, &m_frameBufferID));
      GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferID));

      GLCall(glBindTexture(GL_TEXTURE_2D, m_texture2D->GetID()));
      GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                   GL_TEXTURE_2D,
                             m_texture2D->GetID(), 0));
      GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        return;
    }

}

MotionVectorsFramebuffer::MotionVectorsFramebuffer(int windowWidth, int windowHeight)
    : Framebuffer(windowWidth, windowHeight)
{
    m_texture2D = std::make_unique<MotionTexture2D>(windowWidth, windowHeight);
}

CubemapFramebuffer::CubemapFramebuffer(int windowWidth, int windowHeight)
{
    m_cubemap = std::make_unique<Cubemap>(windowWidth, windowHeight);
    GLCall(glGenFramebuffers(1, &m_frameBufferID));
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferID));
    GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, m_cubemap->GetID(), 0));
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void CubemapFramebuffer::Bind(int textureSlot)
{
  GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferID));
  m_cubemap->Bind(textureSlot);
}

void CubemapFramebuffer::Unbind()
{
  GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
  m_cubemap->Unbind();
}

void CubemapFramebuffer::DrawToFace(int index)
{
    GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + (int)index, m_cubemap->GetID(), 0));
    GLCall(glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT));
}
