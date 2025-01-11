#include "Renderer.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <numeric>

void Renderer::Clear()
{
    GLCall(glClearColor(0.05f, 0.05f, 0.05f, 1.0f));
	GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void Renderer::UpdateViewport(float Width, float Height)
{
    GLCall(glViewport(0, 0, Width, Height));
}

void Renderer::Draw(const VertexArray& va, const IndexBuffer& ib, Shader& shader, bool bShowDebugInfo)
{
    if (bShowDebugInfo) {
        shader.StartTimer();
    }

    shader.Bind();
    va.Bind();
    ib.Bind();

    GLCall(glDrawElements(GL_TRIANGLES, ib.GetCount(), GL_UNSIGNED_INT, nullptr));

    va.Unbind();
    ib.Unbind();

    if (bShowDebugInfo) {
        shader.StopTimer();
        double timeElapsed = shader.GetElapsedTime();

        // Record the time for statistics
        CalculateStatistics(shader.GetName(), timeElapsed);
    }
}

void Renderer::CalculateStatistics(const std::string& shaderName, double elapsedTime)
{
    m_shaderTimes[shaderName].push_back(elapsedTime);
}

void Renderer::SaveStatisticsToFile()
{
    if (!m_debugFilePath.empty()) {
        std::ofstream debugFile(m_debugFilePath, std::ios::out | std::ios::app);

        if (debugFile.is_open()) {
            for (const auto& [shaderName, times] : m_shaderTimes) {
                if (!times.empty()) {
                    double sum = std::accumulate(times.begin(), times.end(), 0.0);
                    double avg = sum / times.size();
                    double minTime = *std::min_element(times.begin(), times.end());
                    double maxTime = *std::max_element(times.begin(), times.end());

                    debugFile << "Shader: " << shaderName << std::endl;
                    debugFile << "  Average Time: " << avg << " ms" << std::endl;
                    debugFile << "  Max Time: " << maxTime << " ms" << std::endl;
                    debugFile << "  Min Time: " << minTime << " ms" << std::endl;
                }
            }

            debugFile.close();
        }
    }
}

void Renderer::CalculateParticleShaderTimes(const std::string& shaderName, double elapsedTime)
{
    m_shaderTimesForParticles[shaderName].push_back(elapsedTime);
}

void Renderer::SaveParticleShaderTimesToFile()
{
    if (!m_debugFilePath.empty()) {
        std::ofstream debugFile(m_debugFilePath, std::ios::out | std::ios::app);

        if (debugFile.is_open()) {
            for (const auto& [shaderName, times] : m_shaderTimesForParticles) {
                if (!times.empty()) {
                    double sum = std::accumulate(times.begin(), times.end(), 0.0);
                    double avg = sum / times.size();
                    double minTime = *std::min_element(times.begin(), times.end());
                    double maxTime = *std::max_element(times.begin(), times.end());

                    debugFile << "Shader: " << shaderName << std::endl;
                    debugFile << "  Average Time: " << avg << " ms" << std::endl;
                    debugFile << "  Max Time: " << maxTime << " ms" << std::endl;
                    debugFile << "  Min Time: " << minTime << " ms" << std::endl;
                }
            }

            debugFile.close();
        }
    }
}

void Renderer::SetDebugInfo(const std::string& debugFilePath)
{
   m_debugFilePath = debugFilePath;
}
