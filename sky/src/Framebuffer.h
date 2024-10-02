#pragma once

#include "Texture2D.h"
#include "Cubemap.h"

class Framebuffer {
public:
  Framebuffer(int windowWidth, int windowHeight);
  virtual ~Framebuffer();

  void Bind(int textureSlot);
  void Unbind();
  void Draw();
  Texture2D &GetTexture() { return *m_texture2D.get(); } 

public:
  unsigned int m_frameBufferID;
  std::unique_ptr<Texture2D> m_texture2D;
};

class DepthFramebuffer : public Framebuffer
{
public:
	DepthFramebuffer(int windowWidth, int windowHeight);
};

class MotionVectorsFramebuffer : public Framebuffer
{
public:
	MotionVectorsFramebuffer(int windowWidth, int windowHeight);
};

class CubemapFramebuffer
{
public:
	CubemapFramebuffer(int windowWidth, int windowHeight);
	void Bind(int textureSlot);
	void Unbind();
	void DrawToFace(int index);
	Cubemap &GetCubemap() { return *m_cubemap.get(); } 
private:
	std::unique_ptr<Cubemap> m_cubemap;
	unsigned int m_frameBufferID;
};

