#pragma once

#include "Util.h"
#include <vector>
#include <iostream>

class Texture3D
{
public:
	Texture3D(): m_rendererID(0), m_localBuffer(nullptr), m_width(0), m_height(0), m_depth(0), m_bGenerateMipmaps(0), bInitialized(false) {}
	Texture3D(const std::string& path, int width, int height, int depth, bool bGenerateMipmaps = false, bool bClampToBorder = false);
	Texture3D(const std::vector<float>& textureBuffer, int width, int height, int depth, bool bGenerateMipmaps = false, bool bClampToBorder = false);
	~Texture3D();

	bool Load(const std::string& path, int width, int height, int depth, bool bGenerateMipmaps = false, bool bClampToBorder = false);
	bool Load(const std::vector<float>& textureBuffer, int width, int height, int depth, bool bGenerateMipmaps = false, bool bClampToBorder = false);
	void Bind(unsigned int slot = 0) const;
	void Unbind() const;

	inline int GetWidth() const { return m_width; }
	inline int GetHeight() const { return m_height; }

private:
	unsigned int m_rendererID;
	unsigned char* m_localBuffer;
	int m_width, m_height, m_depth;
	bool m_bGenerateMipmaps;
	bool bInitialized;
};