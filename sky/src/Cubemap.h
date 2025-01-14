#pragma once
#include "Util.h"
#include <vector>
#include <iostream>

class Cubemap
{
public:
	Cubemap(): m_rendererID(0), m_localBuffer(nullptr), m_width(0), m_height(0), m_depth(0), m_bGenerateMipmaps(0), bInitialized(false), m_lastSlot(-1) {}
	Cubemap(int width, int height);
	virtual ~Cubemap();

	void Bind(unsigned int slot = 0);
	void Unbind();

	inline int GetWidth() const { return m_width; }
	inline int GetHeight() const { return m_height; }
	unsigned int GetID() const { return m_rendererID; }
    unsigned int GetSlot() const { return m_lastSlot; }

private:
	unsigned int m_rendererID;
	unsigned char* m_localBuffer;
	int m_lastSlot = -1;
	int m_width, m_height, m_depth;
	bool m_bGenerateMipmaps;
	bool bInitialized;
};

