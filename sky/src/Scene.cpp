#include "Scene.h"
#include <fstream>
#include "Application.h"
#pragma optimize("", off)
Scene::Scene(const std::string& jsonFilePath) {
    m_scale = glm::vec3(1.0f);
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
    
    // Load global scale if it exists
    if (j.contains("scale") && j["scale"].is_array() && j["scale"].size() == 3) {
        m_scale = glm::vec3(j["scale"][0].as<double>(), j["scale"][1].as<double>(), j["scale"][2].as<double>());
    }

     InitializeModels(j);
     ParseModelParameters(j);
}

void Scene::InitializeModels(const json& j) {
    for (const auto& element : j.object_range()) {
        std::string modelName = element.key();
        json modelParams = element.value();

        // Extract the path to the OBJ file
        if (modelParams.contains("objFilePath") && m_models.size() < Application::MaxNumberOfMeshes) {
            std::string objFilePath = modelParams["objFilePath"].as<std::string>();

            // Check if the model is already cached
            if (m_modelCache.find(objFilePath) == m_modelCache.end()) {
                // If not cached, create a new model and store it in the cache
                m_modelCache[objFilePath] = std::make_shared<Model>(objFilePath, false, this);
            }

            // Assign the cached model to the scene
            m_models[modelName] = m_modelCache[objFilePath];
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
            glm::vec3 collisionSize(0.0f);
            bool hasPosition = false;
            bool hasScale = false;
            bool hasRotation = false;
            bool hasCollisionSize = false;

            for (const auto& param : modelParams.object_range()) {
                std::string paramName = param.key();
                const json& paramValue = param.value();

                if (paramName == "position" && paramValue.is_array() && paramValue.size() == 3) {
                    position = glm::vec3(paramValue[0].as<double>(), paramValue[1].as<double>(), paramValue[2].as<double>()) * m_scale;
                    hasPosition = true;
                } else if (paramName == "scale" && paramValue.is_array() && paramValue.size() == 3) {
                    scale = glm::vec3(paramValue[0].as<double>(), paramValue[1].as<double>(), paramValue[2].as<double>()) * m_scale;
                    hasScale = true;
                } else if (paramName == "rotation" && paramValue.is_array() && paramValue.size() == 3) {
                    rotation = glm::vec3(paramValue[0].as<double>(), paramValue[1].as<double>(), paramValue[2].as<double>());
                    hasRotation = true;
                } else if (paramName == "collisionSize" && paramValue.is_array() && paramValue.size() == 3) {
                    collisionSize = glm::vec3(paramValue[0].as<double>(), paramValue[1].as<double>(), paramValue[2].as<double>());
                    hasCollisionSize = true;
                } else if (paramValue.is<double>()) {
                    SetParameterForModel(modelName, paramName, (float)paramValue.as<double>());
                } else if (paramValue.is<int>()) {
                    SetParameterForModel(modelName, paramName, paramValue.as<int>());
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

            // Store collision size if available
            if (hasCollisionSize) {
                m_models[modelName]->SetCollisionSize(collisionSize);
            }
        }
    }
}

void Scene::SetParameterForModel(const std::string& modelName, const std::string& name, int value) {
    if (m_models.find(modelName) != m_models.end()) {
        m_models[modelName]->SetParameter(name, value);
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
    renderer.SaveParticleShaderTimesToFile();
}

void Scene::Uninitialize()
{
   for (auto& [name, model] : m_models) {
       if(m_models[name])
          m_models[name]->Uninitialize();
  }
}

std::unordered_map<std::string, std::shared_ptr<Texture2D>>& Scene::GetLoadedTexturesMap() 
{
    return loadedTexturesMap;
}

void Scene::Draw(std::unordered_map<std::string, Shader*>& shaders, const PerspectiveCamera& camera) {
    for (auto& [name, shader] : shaders) {
        glm::mat4 modelMatrix = m_models[name]->GetParameterMat4("model");
       // if (m_models[name]->CheckFrustumCull(camera, modelMatrix))
        {
            {
                if (m_bSetDebugInfo) 
                {
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
                } 
                else 
                { 

                    m_models[name]->Draw(*shader, m_scale);
                }
            }

        }
    }
}

std::vector<ObjectProperties> Scene::GetAvailableObjectsProperties() const {
    std::vector<ObjectProperties> objects;
    for (const auto& [modelName, model] : m_models) {
        ObjectProperties properties;
        properties.name = modelName;
        properties.isGlass = model->HasParameter("u_isGlass") && model->GetParameterInt("u_isGlass") == 1;
        properties.hasShadow = model->HasParameter("u_hasShadow") && model->GetParameterInt("u_hasShadow") == 1;
        properties.isGround = model->HasParameter("u_isGround") && model->GetParameterInt("u_isGround") == 1;
        objects.push_back(properties);
    }
    return objects;
}

std::optional<glm::vec3> Scene::GetCollisionSize(const std::string& modelName) const {
    if (m_models.find(modelName) != m_models.end() && m_models.at(modelName)->HasCollisionSize()) {
        return m_models.at(modelName)->GetCollisionSize();
    }
    return std::nullopt; // Return empty if no collision size is defined
}