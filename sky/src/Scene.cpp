#include "Scene.h"
#include <fstream>
#pragma optimize("", off)
Scene::Scene(const std::string& jsonFilePath) {
    LoadParametersFromFile(jsonFilePath);
    m_bSetDebugInfo = false;
}

void Scene::LoadParametersFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open config file");
    }

    json j;
    file >> j;
    
   InitializeModels(j);
   ParseModelParameters(j);
}

void Scene::InitializeModels(const json& j) {
    for (const auto& element : j.object_range()) {
        std::string modelName = element.key();
        json modelParams = element.value();

        // Extract the path to the OBJ file
        if (modelParams.contains("objFilePath")) {
            std::string objFilePath = modelParams["objFilePath"].as<std::string>();

            // Create a Model instance with the OBJ file path
            m_models[modelName] = std::make_unique<Model>(objFilePath);
        }
    }
}

void Scene::ParseModelParameters(const json& j) {
    for (const auto& element : j.object_range()) {
        std::string modelName = element.key();
        json modelParams = element.value();

        // Check if parameters exist for the model
        if (m_models.find(modelName) != m_models.end()) {
            glm::vec3 position(0.0f);
            glm::vec3 scale(1.0f);
            glm::vec3 rotation(0.0f);
            bool hasPosition = false;
            bool hasScale = false;
            bool hasRotation = false;

            for (const auto& param : modelParams.object_range()) {
                std::string paramName = param.key();
                const json& paramValue = param.value();

                if (paramName == "position" && paramValue.is_array() && paramValue.size() == 3) {
                    position = glm::vec3(paramValue[0].as<double>(), paramValue[1].as<double>(), paramValue[2].as<double>());
                    hasPosition = true;
                } else if (paramName == "scale" && paramValue.is_array() && paramValue.size() == 3) {
                    scale = glm::vec3(paramValue[0].as<double>(), paramValue[1].as<double>(), paramValue[2].as<double>());
                    hasScale = true;
                } else if (paramName == "rotation" && paramValue.is_array() && paramValue.size() == 3) {
                    rotation = glm::vec3(paramValue[0].as<double>(), paramValue[1].as<double>(), paramValue[2].as<double>());
                    hasRotation = true;
                } else if (paramValue.is<double>()) {
                    SetParameterForModel(modelName, paramName, paramValue.as<double>());
                } else if (paramValue.is_array() && paramValue.size() == 4) {
                    glm::vec4 vec(paramValue[0].as<double>(), paramValue[1].as<double>(), paramValue[2].as<double>(), paramValue[3].as<double>());
                    SetParameterForModel(modelName, paramName, vec);
                }
            }

            // Apply transformation to the "model" parameter
            if (hasPosition || hasScale || hasRotation) {
                glm::mat4 modelMatrix = glm::mat4(1.0f);

                if (hasPosition) {
                    modelMatrix = glm::translate(modelMatrix, position);
                }

                if (hasRotation) {
                    modelMatrix = glm::rotate(modelMatrix, rotation.x, glm::vec3(1, 0, 0));
                    modelMatrix = glm::rotate(modelMatrix, rotation.y, glm::vec3(0, 1, 0));
                    modelMatrix = glm::rotate(modelMatrix, rotation.z, glm::vec3(0, 0, 1));
                }

                if (hasScale) {
                    modelMatrix = glm::scale(modelMatrix, scale);
                }

                SetParameterForModel(modelName, "model", modelMatrix);
            }
        }
    }
}

void Scene::SetParameterForModel(const std::string& modelName, const std::string& name, float value) {
    if (m_models.find(modelName) != m_models.end()) {
        m_models[modelName]->SetParameter(name, value);
    }
}

void Scene::SetParameterForModel(const std::string& modelName, const std::string& name, const glm::vec3& value) {
    if (m_models.find(modelName) != m_models.end()) {
        m_models[modelName]->SetParameter(name, value);
    }
}

void Scene::SetParameterForModel(const std::string& modelName, const std::string& name, const glm::vec4& value) {
    if (m_models.find(modelName) != m_models.end()) {
        m_models[modelName]->SetParameter(name, value);
    }
}

void Scene::SetParameterForModel(const std::string& modelName, const std::string& name, const glm::mat4& value) {
    if (m_models.find(modelName) != m_models.end()) {
        m_models[modelName]->SetParameter(name, value);
    }
}


Model* Scene::GetModel(const std::string& modelName)
{
    return m_models[modelName].get();
}

void Scene::SetDebugInfo(bool bSetDebugInfo, const std::string& debugFilePath) {
    m_bSetDebugInfo = bSetDebugInfo;

    // If debug info is enabled and a file path is provided, open the file for writing
    if (m_bSetDebugInfo && !debugFilePath.empty()) {
        m_debugFilePath = debugFilePath;
    }

    renderer.SetDebugInfo(debugFilePath);
}

void Scene::SaveStatisticsToFile() {
    if (!m_debugFilePath.empty()) {
        std::ofstream debugFile(m_debugFilePath, std::ios::out | std::ios::app);
        if (!debugFile.is_open()) {
            std::cerr << "Failed to open debug file: " << m_debugFilePath << std::endl;
            return;
        }

        for (const auto& [shaderName, stats] : m_shaderStatistics) {
            if (stats.executionCount > 0) {
                double avgTime = stats.totalTime / stats.executionCount;
                debugFile << "Shader: " << shaderName << std::endl;
                debugFile << "  Average Time: " << avgTime << " ms" << std::endl;
                debugFile << "  Max Time: " << stats.maxTime << " ms" << std::endl;
                debugFile << "  Min Time: " << stats.minTime << " ms" << std::endl;
                debugFile << "  Total Executions: " << stats.executionCount << std::endl;
            }
        }

        debugFile.close();
    }
    renderer.SaveStatisticsToFile();
}

void Scene::Uninitialize()
{
   for (auto& [name, model] : m_models) {
       if(m_models[name])
          m_models[name]->Uninitialize();
  }
}

void Scene::Draw(std::unordered_map<std::string, Shader*>& shaders) {
    for (auto& [name, shader] : shaders) {
        if (m_bSetDebugInfo) {
            shader->StartTimer();
            m_models[name]->Draw(*shader);
            shader->StopTimer();

            double timeElapsed = shader->GetElapsedTime();

            // Update statistics for this shader
            auto& stats = m_shaderStatistics[shader->GetName()];
            stats.totalTime += timeElapsed;
            stats.executionCount++;
            stats.maxTime = std::max(stats.maxTime, timeElapsed);
            stats.minTime = std::min(stats.minTime, timeElapsed);

        } else {
            m_models[name]->Draw(*shader);
        }
    }
}