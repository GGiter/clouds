#pragma once

#include "Util.h"
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"

class Renderer
{
public:
    void Clear();
    void UpdateViewport(float Width, float Height);
    void Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader) const;
};