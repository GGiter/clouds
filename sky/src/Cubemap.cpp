#include "Cubemap.h"
#include "TextureCounter.h"

Cubemap::Cubemap(int width, int height)
	: m_rendererID(0), m_localBuffer(nullptr), m_width(width), m_height(height), m_depth(0), m_bGenerateMipmaps(0), bInitialized(false), m_lastSlot(-1) 
{
	GLCall(glGenTextures(1, &m_rendererID));
	GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, m_rendererID));

	GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

	GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));

	for(unsigned int i = 0; i < 6; ++i)
	{
		GLCall(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X  + i, 0, GL_RGBA32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, 0));
	}

	GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));

	bInitialized = true;
}

Cubemap::~Cubemap()
{
	GLCall(glDeleteTextures(1, &m_rendererID));
}

void Cubemap::Bind(unsigned int slot)
{
	if(m_lastSlot != -1)
	{
		TextureCounter::ReleaseID(m_lastSlot);
	}
	m_lastSlot = (int)slot;
	GLCall(glActiveTexture(GL_TEXTURE0 + slot));
	GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, m_rendererID));
}

void Cubemap::Unbind()
{
	if(m_lastSlot != -1)
	{
		TextureCounter::ReleaseID(m_lastSlot);
	}
	m_lastSlot = -1;
	GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
}
