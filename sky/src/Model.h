#include <glad/glad.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#pragma once

#include <assimp/postprocess.h>
#include <unordered_map>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include "Mesh.h"
#include "PerspectiveCamera.h"

class Texture2D;
class Shader;
class Scene;

class Model 
{
public:
    // model data 
    std::vector<std::shared_ptr<Texture2D>> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    std::vector<std::unique_ptr<Mesh>>    meshes;
    std::string directory;
    bool gammaCorrection;
    glm::vec3 m_collisionSize;

    // constructor, expects a filepath to a 3D model.
    Model(std::string const &path, bool gamma = false, Scene* InScene = nullptr);
    virtual ~Model();
    // draws the model, and thus all its meshes
    void Draw(Shader &shader,const glm::vec3& sceneScale = glm::vec3(1.f));

 
    void SetParameter(const std::string& name, int value);
    void SetParameter(const std::string& name, float value);
    void SetParameter(const std::string& name, const glm::vec3& value);
    void SetParameter(const std::string& name, const glm::mat4& value);

    bool HasParameter(const std::string& name) const;
    int GetParameterInt(const std::string& name) const;
    float GetParameterFloat(const std::string& name) const;
    glm::vec3 GetParameterVec3(const std::string& name) const;
    glm::mat4 GetParameterMat4(const std::string& name) const;
    void SetCollisionSize(const glm::vec3 &size);
    glm::vec3 GetCollisionSize() const;
    bool HasCollisionSize() const;
    glm::vec3 GetBoundsMin() const;
    glm::vec3 GetBoundsMax() const;
    glm::vec3 GetBoundsCenter() const;
    glm::vec3 GetBoundsSize() const;
    bool CheckFrustumCull(const PerspectiveCamera &camera,
                          const glm::mat4 &modelMatrix) const;
    void Uninitialize();
    
private:
    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(std::string const &path, Scene* InScene);
    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode *node, const aiScene *scene, Scene* InScene);

    std::unique_ptr<Mesh> processMesh(aiMesh *mesh, const aiScene *scene, Scene* InScene);

    // checks all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a Texture struct.
    std::vector<std::shared_ptr<Texture2D>>
    loadMaterialTextures(aiMaterial *mat, aiTextureType type,
                         std::string typeName, Scene* InScene);

    std::unordered_map<std::string, int> intParams;
    std::unordered_map<std::string, float> floatParams;
    std::unordered_map<std::string, glm::vec3> vec3Params;
    std::unordered_map<std::string, glm::mat4> mat4Params;
    bool boundsInitialized;
    glm::vec3 boundsMin;
    glm::vec3 boundsMax;
};