#include "Texture2D.h"
#include "stb_image/stb_image.h"
#include "TextureCounter.h"
#include <FreeImage.h>
#include <algorithm>
#include <gli/gli.hpp>

Texture2D::Texture2D(const std::string &path, int width, int height, bool bClampToEdge)
{
  bInitialized = false;
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
	else
	{
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
	}

	GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, nullptr));
	GLCall(glGenerateMipmap(GL_TEXTURE_2D));
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));

	bInitialized = true;
}

void Texture2D::LoadTexture(const std::string& path)
{
    if (path.empty())
    {
        std::cerr << "Path to texture file is empty." << std::endl;
        return;
    }

    // Check if the file extension is DDS
    std::string extension = path.substr(path.find_last_of('.') + 1);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower); // Convert extension to lowercase for case-insensitive comparison

    if (extension == "dds")
    {
        // Use gli to load DDS file
        gli::texture texture = gli::load(path);
        if (texture.empty())
        {
            std::cerr << "Failed to load DDS texture: " << path << std::endl;
            return;
        }

        gli::texture2d texture2D(texture);
        if (texture2D.empty())
        {
            std::cerr << "Failed to convert DDS texture to 2D texture: " << path << std::endl;
            return;
        }

        // Extract texture properties
        m_width = static_cast<int>(texture2D.extent().x);
        m_height = static_cast<int>(texture2D.extent().y);

		// Perform conversion to 32-bit RGBA
		gli::texture2d converted = gli::convert(texture2D, gli::FORMAT_RGBA8_UNORM_PACK8);
		if (converted.empty())
		{
			std::cerr << "Failed to convert texture to 32-bit RGBA: " << path << std::endl;
			return;
		}

		texture2D = converted;

        m_BPP = 4;

        // Allocate memory for local buffer
        size_t bufferSize = texture2D.size();
        m_localBuffer = new unsigned char[bufferSize];

        // Copy texture data
        std::memcpy(m_localBuffer, texture2D.data(0, 0, 0), bufferSize);
    }
    else
    {
        // Use stb_image to load non-DDS files
		stbi_set_flip_vertically_on_load(1);
        m_localBuffer = stbi_load(path.c_str(), &m_width, &m_height, &m_BPP, 0);
        if (!m_localBuffer)
        {
            std::cerr << "Failed to load texture file: " << path << " with stb_image." << std::endl;
        }
    }
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

	LoadTexture(path);

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


	std::string extension = path.substr(path.find_last_of('.') + 1);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

	if (extension != "dds")
	{
		if (m_localBuffer)
			stbi_image_free(m_localBuffer);
	}
	else
	{
		delete m_localBuffer;
		m_localBuffer = nullptr;
	}
					

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
