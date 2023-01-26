#pragma once

#include "Texture2D.h"

class Framebuffer {
public:
  Framebuffer(int windowWidth, int windowHeight);
  ~Framebuffer();

  void Bind(int textureSlot);
  void Unbind();
  void Draw();
  Texture2D &GetTexture() { return m_texture2D; } 

public:
  unsigned int m_frameBufferID;
  Texture2D m_texture2D;
};
