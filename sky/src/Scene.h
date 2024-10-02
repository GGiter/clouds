#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>
#include <jsoncons/json.hpp>
#include "Model.h"
#include "Shader.h"
#include "Renderer.h"

using jsoncons::json;

struct ShaderStats {
    double totalTime = 0.0;
    double maxTime = 0.0;
    double minTime = std::numeric_limits<double>::max();
    size_t executionCount = 0;
};

class Scene {
public:
    Scene(const std::string& jsonFilePath);
    virtual ~Scene() {};
    void LoadParametersFromFile(const std::string& filePath);
    void Draw(std::unordered_map<std::string, Shader*>& shaders);
    void SetParameterForModel(const std::string& modelName, const std::string& name, float value);
    void SetParameterForModel(const std::string& modelName, const std::string& name, const glm::vec3& value);
    void SetParameterForModel(const std::string& modelName, const std::string& name, const glm::vec4& value);
    void SetParameterForModel(const std::string& modelName, const std::string& name, const glm::mat4& value);
    Model* GetModel(const std::string& modelName);
    void SetDebugInfo(bool bSetDebugInfo, const std::string& debugFilePath = "");
    void SaveStatisticsToFile();
    Renderer& GetRenderer() { return renderer; }
    void Uninitialize();
private:
    Renderer renderer;
    std::unordered_map<std::string, std::unique_ptr<Model>> m_models;
    std::unordered_map<std::string, ShaderStats> m_shaderStatistics;
    bool m_bSetDebugInfo;
    std::string m_debugFilePath;
    void ParseModelParameters(const json& j);
    void InitializeModels(const json& j);
};
