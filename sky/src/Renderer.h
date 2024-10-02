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
    void Draw(const VertexArray& va, const IndexBuffer& ib, Shader& shader,bool bShowDebugInfo = false);
    void SaveStatisticsToFile();
    void SetDebugInfo(const std::string& debugFilePath);
private:
    std::string m_debugFilePath;
    std::unordered_map<std::string, std::vector<double>> m_shaderTimes;
    void CalculateStatistics(const std::string& shaderName, double elapsedTime);
};