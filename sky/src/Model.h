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

class Texture2D;
class Shader;

class Model 
{
public:
    // model data 
    std::vector<std::unique_ptr<Texture2D>> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    std::vector<std::unique_ptr<Mesh>>    meshes;
    std::string directory;
    bool gammaCorrection;

    // constructor, expects a filepath to a 3D model.
    Model(std::string const &path, bool gamma = false);
    virtual ~Model();
    // draws the model, and thus all its meshes
    void Draw(Shader &shader);

    void SetParameter(const std::string& name, float value);
    void SetParameter(const std::string& name, const glm::vec3& value);
    void SetParameter(const std::string& name, const glm::mat4& value);
    void Uninitialize();
    
private:
    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(std::string const &path);

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode *node, const aiScene *scene);

    std::unique_ptr<Mesh> processMesh(aiMesh *mesh, const aiScene *scene);

    // checks all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a Texture struct.
    std::vector<std::unique_ptr<Texture2D>> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);

    std::unordered_map<std::string, float> floatParams;
    std::unordered_map<std::string, glm::vec3> vec3Params;
    std::unordered_map<std::string, glm::mat4> mat4Params;
};