#pragma once

#include "Util.h"
#include <vector>
#include <iostream>

class Texture2D
{
public:
	Texture2D() : bInitialized(false), m_BPP(0), m_localBuffer(nullptr), m_width(0), m_height(0), m_rendererID(0)  {}
	Texture2D(const std::string& path, int width = 0, int height = 0, bool bClampToEdge = false);
	Texture2D(const std::vector<float>& textureBuffer, int width, int height, bool bClampToEdge = false);
	~Texture2D();

	bool Load(const std::string& path, int width = 0, int height = 0, bool bClampToEdge = false);
	void Bind(unsigned int slot = 0) const;
	void Unbind() const;

	inline int GetWidth() const { return m_width; }
	inline int GetHeight() const { return m_height; }
    unsigned int GetID() const { return m_rendererID; }
private:
    unsigned int m_rendererID;
	std::string m_filePath;
	unsigned char* m_localBuffer;
	int m_width, m_height, m_BPP;
    bool bInitialized;
};