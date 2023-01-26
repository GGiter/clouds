#include "Framebuffer.h"

Framebuffer::Framebuffer(int windowWidth, int windowHeight) 
    :m_texture2D("", windowWidth, windowHeight) 
{
  GLCall(glGenFramebuffers(1, &m_frameBufferID));
  GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferID));

  GLCall(glBindTexture(GL_TEXTURE_2D, m_texture2D.GetID()));
  GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D,
                         m_texture2D.GetID(), 0));
  GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

Framebuffer::~Framebuffer() 
{
}

void Framebuffer::Bind(int textureSlot) 
{
  GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferID));
  m_texture2D.Bind(textureSlot);
}

void Framebuffer::Unbind()
{ 
  GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
  m_texture2D.Unbind();
}

void Framebuffer::Draw() 
{
  GLenum bufferlist[] = {GL_COLOR_ATTACHMENT0};
  GLCall(glDrawBuffers(1, bufferlist));
}
