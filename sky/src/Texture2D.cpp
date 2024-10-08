#include "Texture2D.h"
#include "stb_image/stb_image.h"
#include "TextureCounter.h"

Texture2D::Texture2D(const std::string &path, int width, int height, bool bClampToEdge)
{
	Load(path, width, height, bClampToEdge);
}

Texture2D::Texture2D(const std::vector<float>& textureBuffer, int width, int height, bool bClampToEdge)
 : m_localBuffer(nullptr), m_width(width), m_height(height), m_BPP(0) , m_lastSlot(-1)
{
	GLCall(glGenTextures(1, &m_rendererID));
	GLCall(glBindTexture(GL_TEXTURE_2D, m_rendererID));

	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

	if(bClampToEdge)
	{
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	}

	GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, textureBuffer.data()));
	GLCall(glGenerateMipmap(GL_TEXTURE_2D));
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));

	bInitialized = true;
}

Texture2D::Texture2D(int width, int height, bool bClampToEdge)
 : m_localBuffer(nullptr), m_width(width), m_height(height), m_BPP(0) , m_lastSlot(-1)
{
	GLCall(glGenTextures(1, &m_rendererID));
	GLCall(glBindTexture(GL_TEXTURE_2D, m_rendererID));

	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

	if(bClampToEdge)
	{
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	}

	GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, nullptr));
	GLCall(glGenerateMipmap(GL_TEXTURE_2D));
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));

	bInitialized = true;
}

Texture2D::~Texture2D()
{
  if (bInitialized) 
  {
    GLCall(glDeleteTextures(1, &m_rendererID));
  }
}

bool Texture2D::Load(const std::string& path, int width, int height, bool bClampToEdge)
{
	if(bInitialized)
		return false;

	m_filePath = path;
	m_localBuffer = nullptr;
	m_width = width;
	m_height = height;
	m_BPP = 0;
	stbi_set_flip_vertically_on_load(1);
	if (path != "")
	m_localBuffer = stbi_load(path.c_str(), &m_width, &m_height, &m_BPP, 0);

	GLCall(glGenTextures(1, &m_rendererID));
	GLCall(glBindTexture(GL_TEXTURE_2D, m_rendererID));

	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

	if(bClampToEdge)
	{
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	}
	else
	{
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
	}

	GLCall(glTexImage2D(GL_TEXTURE_2D, 0, m_BPP > 3 ? GL_RGBA8 : GL_RGB, m_width, m_height, 0, m_BPP > 3 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, m_localBuffer));
	GLCall(glGenerateMipmap(GL_TEXTURE_2D));
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));

	if (m_localBuffer)
		stbi_image_free(m_localBuffer);

	bInitialized = true;

	return true;
}

void Texture2D::Bind(unsigned int slot)
{
	if(m_lastSlot != -1)
	{
		TextureCounter::ReleaseID(m_lastSlot);
	}
	m_lastSlot = (int)slot;
	GLCall(glActiveTexture(GL_TEXTURE0 + slot));
	GLCall(glBindTexture(GL_TEXTURE_2D, m_rendererID));
}

void Texture2D::Unbind()
{
	if(m_lastSlot != -1)
	{
		TextureCounter::ReleaseID(m_lastSlot);
	}
	m_lastSlot = -1;
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));
}

DepthTexture2D::DepthTexture2D(int width, int height, bool bClampToEdge)
	: Texture2D(width, height, bClampToEdge)
{
	GLCall(glGenTextures(1, &m_rendererID));
	GLCall(glBindTexture(GL_TEXTURE_2D, m_rendererID));

	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

	GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr));
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));

	bInitialized = true;
}

MotionTexture2D::MotionTexture2D(int width, int height, bool bClampToEdge)
	: Texture2D(width, height, bClampToEdge)
{
	GLCall(glGenTextures(1, &m_rendererID));
	GLCall(glBindTexture(GL_TEXTURE_2D, m_rendererID));

	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

	if(bClampToEdge)
	{
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	}

	GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, m_width, m_height, 0, GL_RG, GL_HALF_FLOAT, nullptr));
	GLCall(glGenerateMipmap(GL_TEXTURE_2D));
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));

	bInitialized = true;
}
