#include "Model.h"
#include "Shader.h"
#include "Texture2D.h"
#include "Util.h"
#include "Scene.h"
#include "Application.h"
#include "PerspectiveCamera.h"
#include <limits>
#pragma optimize("", off)
unsigned int TextureFromFile(const char *path, const std::string &directory, bool gamma)
{
    std::string filename = std::string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

Model::Model(std::string const &path, bool gamma, Scene* InScene)
    : gammaCorrection(gamma), 
      m_collisionSize(glm::vec3(0.0f)), 
      boundsInitialized(false),
      boundsMin(glm::vec3(std::numeric_limits<float>::max())), 
      boundsMax(glm::vec3(std::numeric_limits<float>::lowest()))  {
    loadModel(path, InScene);
}

Model::~Model()
{
}

void Model::Draw(Shader &shader, const glm::vec3& sceneScale)
{
    for (const auto& [key, value] : intParams) {
        shader.SetUniform1i(key.c_str(), value);
    }
    for (const auto& [key, value] : floatParams) {
        shader.SetUniform1f(key.c_str(), value);
    }
    for (const auto& [key, value] : vec3Params) {
        shader.SetUniformVec3f(key.c_str(), value);
    }
    for (const auto& [key, value] : mat4Params) {
        shader.SetUniformMat4f(key.c_str(), value);
    }

    for(unsigned int i = 0; i < meshes.size(); i++)
        meshes[i]->Draw(shader, sceneScale);
}
    
void Model::loadModel(std::string const &path, Scene* InScene)
{
    // read file via ASSIMP
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    // check for errors
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
    {
        std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return;
    }
    // retrieve the directory path of the filepath
    directory = path.substr(0, path.find_last_of('/'));

    // process ASSIMP's root node recursively
    processNode(scene->mRootNode, scene, InScene);
}

void Model::SetParameter(const std::string& name, int value) {
    intParams[name] = value;
}

void Model::SetParameter(const std::string& name, float value) {
    floatParams[name] = value;
}

void Model::SetParameter(const std::string& name, const glm::vec3& value) {
    vec3Params[name] = value;
}

void Model::SetParameter(const std::string& name, const glm::mat4& value) {
    mat4Params[name] = value;
}

bool Model::HasParameter(const std::string& name) const {
    return intParams.find(name) != intParams.end() ||
           floatParams.find(name) != floatParams.end() ||
           vec3Params.find(name) != vec3Params.end() ||
           mat4Params.find(name) != mat4Params.end();
}

int Model::GetParameterInt(const std::string& name) const {
    auto it = intParams.find(name);
    if (it != intParams.end()) {
        return it->second;
    }
    throw std::runtime_error("Integer parameter '" + name + "' not found.");
}

float Model::GetParameterFloat(const std::string& name) const {
    auto it = floatParams.find(name);
    if (it != floatParams.end()) {
        return it->second;
    }
    throw std::runtime_error("Float parameter '" + name + "' not found.");
}

glm::vec3 Model::GetParameterVec3(const std::string& name) const {
    auto it = vec3Params.find(name);
    if (it != vec3Params.end()) {
        return it->second;
    }
    throw std::runtime_error("Vec3 parameter '" + name + "' not found.");
}

glm::mat4 Model::GetParameterMat4(const std::string& name) const {
    auto it = mat4Params.find(name);
    if (it != mat4Params.end()) {
        return it->second;
    }
    throw std::runtime_error("Mat4 parameter '" + name + "' not found.");
}

void Model::Uninitialize()
{
    for (const auto& meshPtr : meshes)
    {
        if (meshPtr)
        {
            meshPtr->Uninitialize();
        }
    }
}

// processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
void Model::processNode(aiNode *node, const aiScene *scene, Scene* InScene)
{
    // process each mesh located at the current node
    assert(node->mNumMeshes > 1);
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        // the node object only contains indices to index the actual objects in the scene. 
        // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(std::move(processMesh(mesh, scene, InScene)));
    }
    // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene, InScene);
    }

}

std::unique_ptr<Mesh> Model::processMesh(aiMesh *mesh, const aiScene *scene, Scene* InScene)
{
    // data to fill
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<std::shared_ptr<Texture2D>> textures;

    glm::vec3 localMin(std::numeric_limits<float>::max());
    glm::vec3 localMax(std::numeric_limits<float>::lowest());

    // walk through each of the mesh's vertices
    for(unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
        // positions
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.position = vector;

        // Update local mesh bounds
        localMin = glm::min(localMin, vector);
        localMax = glm::max(localMax, vector);

        // normals
        if (mesh->HasNormals())
        {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.normal = vector;
        }
        // texture coordinates
        if(mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            glm::vec2 vec;
            // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
            // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
            vec.x = mesh->mTextureCoords[0][i].x; 
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
            // tangent
            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.Tangent = vector;
            // bitangent
            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;
            vertex.Bitangent = vector;
        }
        else
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);

        if(mesh->mColors[0]) // does the mesh contain vertex colors ?
        {
            glm::vec4 color;
            color.x = mesh->mColors[0][i].r;
            color.y = mesh->mColors[0][i].g;
            color.z = mesh->mColors[0][i].b;
            color.w = mesh->mColors[0][i].a;
            vertex.VertexColors = color;
        }

        vertices.push_back(vertex);
    }

    // Update model's bounds with local mesh bounds
    if (!boundsInitialized) {
        boundsMin = localMin;
        boundsMax = localMax;
        boundsInitialized = true;
    } else {
        boundsMin = glm::min(boundsMin, localMin);
        boundsMax = glm::max(boundsMax, localMax);
    }

    // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
    for(unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        // retrieve all indices of the face and store them in the indices vector
        for(unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);        
    }
    // process materials
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];    
    // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
    // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
    // Same applies to other texture as the following list summarizes:
    // diffuse: texture_diffuseN
    // specular: texture_specularN
    // normal: texture_normalN

    // 1. diffuse maps
    std::vector<std::shared_ptr<Texture2D>> diffuseMaps = (loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", InScene));
    for(std::shared_ptr<Texture2D>& texture : diffuseMaps)
        textures.push_back(texture);
    if (!Application::LoadOnlyDiffuseTextures)
    {
        // 2. specular maps
        std::vector<std::shared_ptr<Texture2D>> specularMaps = (loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular", InScene));
        for(std::shared_ptr<Texture2D>& texture : specularMaps)
            textures.push_back(texture);
        // 3. normal maps
        std::vector<std::shared_ptr<Texture2D>> normalMaps = (loadMaterialTextures(material, aiTextureType_NORMALS, "texture_normal", InScene));
        for(std::shared_ptr<Texture2D>& texture : normalMaps)
            textures.push_back(texture);
        // 4. height maps
        std::vector<std::shared_ptr<Texture2D>> heightMaps = (loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_height", InScene));
        for(std::shared_ptr<Texture2D>& texture : heightMaps)
            textures.push_back(texture);
    }
    // return a mesh object created from the extracted mesh data
    return std::make_unique<Mesh>(vertices, indices, textures);
}

// checks all material textures of a given type and loads the textures if they're not loaded yet.
// the required info is returned as a Texture struct.
std::vector<std::shared_ptr<Texture2D>> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName, Scene* InScene)
{
   std::vector<std::shared_ptr<Texture2D>> textures;

    // Get a reference to the global texture map
    auto& loadedTexturesMap = InScene->GetLoadedTexturesMap();

    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);  // Get texture path

        // Check if the texture is already loaded
        auto it = loadedTexturesMap.find(str.C_Str());
        if (it != loadedTexturesMap.end()) {
           // Texture is already loaded, reuse it
            textures.push_back(it->second);
        } else 
        {

           // Store in global map for reuse
          loadedTexturesMap[str.C_Str()] = std::make_shared<Texture2D>(this->directory + "/" + str.C_Str());
          loadedTexturesMap[str.C_Str()]->SetName(typeName);

           // Add to the textures list to return
          textures.push_back(loadedTexturesMap[str.C_Str()]);
        }
    }

    return textures;
}

void Model::SetCollisionSize(const glm::vec3& size) 
{
    m_collisionSize = size;
}

glm::vec3 Model::GetCollisionSize() const 
{
    return m_collisionSize;
}

bool Model::HasCollisionSize() const 
{
    return m_collisionSize != glm::vec3(0.0f); // Non-zero size indicates a collision box
}

glm::vec3 Model::GetBoundsMin() const {
    return boundsMin;
}

glm::vec3 Model::GetBoundsMax() const {
    return boundsMax;
}

glm::vec3 Model::GetBoundsCenter() const {
    return (boundsMin + boundsMax) * 0.5f;
}

glm::vec3 Model::GetBoundsSize() const {
    return boundsMax - boundsMin;
}

bool Model::CheckFrustumCull(const PerspectiveCamera& camera, const glm::mat4& modelMatrix) const {
    // Transform the bounds (min and max) using the model matrix
    glm::vec3 worldCenter = glm::vec3(modelMatrix * glm::vec4(GetBoundsCenter(), 1.0f));
    glm::vec3 worldSize = GetBoundsSize(); // No need to scale, just use the size as it is.

    // Use IsBoxInFrustum with the transformed center and half extents (size / 2)
    return camera.IsBoxInFrustum(worldCenter, worldSize * 0.5f, modelMatrix);
}