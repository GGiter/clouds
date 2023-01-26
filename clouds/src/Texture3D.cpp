#include "Texture3D.h"
#include "stb_image/stb_image.h"

Texture3D::Texture3D(const std::string& path, int width, int height, int depth, bool bGenerateMipmaps, bool bClampToBorder)
{
	Load(path, width, height, depth, bGenerateMipmaps, bClampToBorder);
}

Texture3D::Texture3D(const std::vector<float> &textureBuffer, int width,
                     int height, int depth, bool bGenerateMipmaps,
                     bool bClampToBorder)
{
	Load(textureBuffer, width, height, depth, bGenerateMipmaps, bClampToBorder);
}

Texture3D::~Texture3D()
{
	GLCall(glDeleteTextures(1, &m_rendererID));
}

bool Texture3D::Load(const std::string& path, int width, int height, int depth, bool bGenerateMipmaps, bool bClampToBorder)
{
	if(bInitialized)
		return false;
	int x,y,z;
	unsigned char *textureBuffer = stbi_load(path.c_str(), &x, &y, &z, 0);
	GLCall(glGenTextures(1, &m_rendererID));
	GLCall(glBindTexture(GL_TEXTURE_3D, m_rendererID));

	if (bClampToBorder)
	{
          GLCall(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S,
                                 GL_CLAMP_TO_BORDER));
          GLCall(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T,
                                 GL_CLAMP_TO_BORDER));
          GLCall(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R,
                                 GL_CLAMP_TO_BORDER));
	}
	else
	{
		  GLCall(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S,
                                 GL_REPEAT));
          GLCall(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T,
                                 GL_REPEAT));
          GLCall(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R,
                                 GL_REPEAT));
	}

    GLCall(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER,
                           GL_LINEAR_MIPMAP_LINEAR));
	GLCall(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

	GLCall(glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, width, height, depth, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureBuffer));

	if (bGenerateMipmaps)
		GLCall(glGenerateMipmap(GL_TEXTURE_3D));

	GLCall(glGenerateMipmap(GL_TEXTURE_3D));
	GLCall(glBindTexture(GL_TEXTURE_3D, 0));

	stbi_image_free(textureBuffer);

	bInitialized = true;

	return true;
}

bool Texture3D::Load(const std::vector<float>& textureBuffer, int width, int height, int depth, bool bGenerateMipmaps, bool bClampToBorder)
{
	if(bInitialized)
		return false;

	m_width = width;
	m_height = height;
	m_depth = depth;
	m_bGenerateMipmaps = bGenerateMipmaps;

	GLCall(glGenTextures(1, &m_rendererID));
	GLCall(glBindTexture(GL_TEXTURE_3D, m_rendererID));

	if (bClampToBorder)
	{
          GLCall(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S,
                                 GL_CLAMP_TO_BORDER));
          GLCall(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T,
                                 GL_CLAMP_TO_BORDER));
          GLCall(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R,
                                 GL_CLAMP_TO_BORDER));
	}

    GLCall(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER,
                           GL_LINEAR_MIPMAP_LINEAR));
	GLCall(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

	GLCall(glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, width, height, depth, 0, GL_RGBA, GL_FLOAT, textureBuffer.data()));

	if (bGenerateMipmaps)
		GLCall(glGenerateMipmap(GL_TEXTURE_3D));

	GLCall(glGenerateMipmap(GL_TEXTURE_3D));
	GLCall(glBindTexture(GL_TEXTURE_3D, 0));

	bInitialized = true;

	return true;
}

void Texture3D::Bind(unsigned int slot) const
{
	GLCall(glActiveTexture(GL_TEXTURE0 + slot));
	GLCall(glBindTexture(GL_TEXTURE_3D, m_rendererID));
}

void Texture3D::Unbind() const
{
	GLCall(glBindTexture(GL_TEXTURE_3D, 0));
}
