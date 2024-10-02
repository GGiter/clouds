#pragma once

#include "Util.h"
#include <vector>
#include <iostream>

class Texture2D
{
public:
	Texture2D() : bInitialized(false), m_BPP(0), m_localBuffer(nullptr), m_width(0), m_height(0), m_rendererID(0), m_lastSlot(-1)  {}
	Texture2D(const std::string& path, int width = 0, int height = 0, bool bClampToEdge = false);
	Texture2D(const std::vector<float>& textureBuffer, int width, int height, bool bClampToEdge = false);
	Texture2D(int width, int height, bool bClampToEdge = false);
	virtual ~Texture2D();

	bool Load(const std::string& path, int width = 0, int height = 0, bool bClampToEdge = false);
	void Bind(unsigned int slot = 0);
	void Unbind();

	inline int GetWidth() const { return m_width; }
	inline int GetHeight() const { return m_height; }
    unsigned int GetID() const { return m_rendererID; }
    unsigned int GetSlot() const { return m_lastSlot; }
	std::string GetFilePath() const { return m_filePath; }
	void SetName(const std::string& name) { m_name = name;}
	std::string GetName() const { return m_name; }


protected:
    unsigned int m_rendererID;
    int m_lastSlot = -1;
	std::string m_filePath;
	unsigned char* m_localBuffer;
	int m_width, m_height, m_BPP;
    bool bInitialized;
	std::string m_name; //only used for meshes
};

class DepthTexture2D : public Texture2D
{
public:
	DepthTexture2D(int width, int height, bool bClampToEdge = false);
};

class MotionTexture2D : public Texture2D
{
public:
	MotionTexture2D(int width, int height, bool bClampToEdge = false);
};