#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>
#include <jsoncons/json.hpp>
#include "Model.h"
#include "Shader.h"
#include "Renderer.h"
#include "PerspectiveCamera.h"

using jsoncons::json;

struct ObjectProperties {
    std::string name;
    bool isGlass;
    bool isGround;
    bool hasShadow;
};\

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
    void Draw(std::unordered_map<std::string, Shader *> &shaders,
              const PerspectiveCamera &camera);
    std::vector<ObjectProperties> GetAvailableObjectsProperties() const;
    std::optional<glm::vec3> GetCollisionSize(const std::string &modelName) const;
    void SetParameterForModel(const std::string& modelName, const std::string& name, float value);
    void SetParameterForModel(const std::string& modelName, const std::string& name, const glm::vec3& value);
    void SetParameterForModel(const std::string& modelName, const std::string& name, const glm::vec4& value);
    void SetParameterForModel(const std::string& modelName, const std::string& name, const glm::mat4& value);
    Model* GetModel(const std::string& modelName);
    void SetDebugInfo(bool bSetDebugInfo, const std::string& debugFilePath = "");
    void SaveStatisticsToFile();
    Renderer& GetRenderer() { return renderer; }
    void Uninitialize();
    std::unordered_map<std::string, std::shared_ptr<Texture2D>>& GetLoadedTexturesMap();
    std::unordered_map<std::string, std::shared_ptr<Model>> m_models;
private:
    Renderer renderer;
     std::unordered_map<std::string, std::shared_ptr<Model>> m_modelCache;
    std::unordered_map<std::string, ShaderStats> m_shaderStatistics;
    bool m_bSetDebugInfo;
    std::string m_debugFilePath;
    glm::vec3 m_scale;
    void ParseModelParameters(const json& j);
    void SetParameterForModel(const std::string& modelName, const std::string& name, int value);
    void InitializeModels(const json& j);
     std::unordered_map<std::string, std::shared_ptr<Texture2D>> loadedTexturesMap;
};
