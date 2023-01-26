#pragma once
#include <string>
#include <unordered_map>

#include "glm/glm.hpp"

class Shader
{
public:
	Shader(const std::string& vertexFilePath,const std::string& fragmentFilePath);
	~Shader();

	void Bind() const;
	void Unbind() const;

	void SetUniform1i(const std::string& name, int value);
	void SetUniform1f(const std::string& name, float value);
	void SetUniformMat4f(const std::string& name, const glm::mat4& matrix);
	void SetUniform4f(const std::string& name, float v0, float v1, float f2, float f3);
	void SetUniform2f(const std::string& name, float v0, float v1);
	void SetUniformVec3f(const std::string& name, const glm::vec3& vector);

private:
	int GetUniformLocation(const std::string& name);
	std::string ReadShader(const std::string filePath);
	unsigned int CompileShader(unsigned int type, const std::string& source);
	unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader);

	unsigned int m_RendererID;
	std::string m_vertexFilePath;
	std::string m_fragmentFilePath;
	std::unordered_map<std::string, int> m_UniformLocationCache;
};