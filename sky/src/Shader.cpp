#include "Shader.h"
#include "Util.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <malloc.h>
#include "Shadinclude.h"
#pragma optimize("", off)
Shader::Shader(const std::string& vertexFilePath, const std::string& fragmentFilePath)
	: m_vertexFilePath(vertexFilePath), m_fragmentFilePath(fragmentFilePath), m_RendererID(0), m_queryID(0), m_elapsedTime(0.0)
{
    m_RendererID = CreateShader(Shadinclude::load(vertexFilePath), Shadinclude::load(fragmentFilePath));
    FindFileName();
}

Shader::Shader(const std::string& vertexFilePath, const std::string& fragmentFilePath, const std::string& geometryShader)
	: m_vertexFilePath(vertexFilePath), m_fragmentFilePath(fragmentFilePath), m_RendererID(0) , m_queryID(0), m_elapsedTime(0.0)
{
     m_RendererID = CreateShader(Shadinclude::load(vertexFilePath), Shadinclude::load(fragmentFilePath), Shadinclude::load(geometryShader));
     FindFileName();
}

Shader::~Shader()
{
    Unbind();
    GLCall(glDeleteProgram(m_RendererID));
}

void Shader::Bind()
{
    GLCall(glUseProgram(m_RendererID));
}

void Shader::Unbind()
{
    GLCall(glUseProgram(0));
}

void Shader::SetUniform4f(const std::string& name, float v0, float v1, float f2, float f3)
{
    GLCall(glUniform4f(GetUniformLocation(name), v0, v1, f2, f3));
}

void Shader::SetUniform2f(const std::string& name, float v0, float v1)
{
    GLCall(glUniform2f(GetUniformLocation(name), v0, v1));
}

void Shader::SetUniformVec3f(const std::string& name, const glm::vec3& vector)
{
    GLCall(glUniform3f(GetUniformLocation(name), vector.x, vector.y, vector.z));
}

void Shader::SetUniform1i(const std::string& name, int value)
{
    GLCall(glUniform1i(GetUniformLocation(name), value));
}

void Shader::SetUniform1f(const std::string& name, float value)
{
    GLCall(glUniform1f(GetUniformLocation(name), value));
}

void Shader::SetUniformMat4f(const std::string& name, const glm::mat4& matrix)
{
    GLCall(glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]));
}

int Shader::GetUniformLocation(const std::string& name)
{
    if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
        return m_UniformLocationCache[name];

    GLCall(int location = glGetUniformLocation(m_RendererID, name.c_str()));

    if (location == -1)
        std::cout << "Warning: uniform '" << name << "' doesn't exist!" << std::endl;

    m_UniformLocationCache[name] = location;
    return location;
}

std::string Shader::ReadShader(const std::string filePath)
{
    std::ifstream f(filePath);
    std::string str;
    if (f)
    {
        std::ostringstream ss;
        ss << f.rdbuf();
        str = ss.str();
    }
    return str;
}

unsigned int Shader::CompileShader(unsigned int type, const std::string& source)
{
    GLCall(unsigned int id = glCreateShader(type));
    const char* src = source.c_str();
    GLCall(glShaderSource(id, 1, &src, nullptr));
    GLCall(glCompileShader(id));

    int result;
    GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
    if (result == GL_FALSE)
    {
        int length;
        GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
        char* message = (char*)_malloca(length * sizeof(char));
        GLCall(glGetShaderInfoLog(id, length, &length, message));
        std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : type == GL_FRAGMENT_SHADER ? "fragment" : "geometry") << " shader!" << std::endl;
        std::cout << message << std::endl;
        GLCall(glDeleteShader(id));
        return 0;
    }

    return id;
}

unsigned int Shader::CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
    return CreateShader(vertexShader, fragmentShader, "");
}

unsigned int Shader::CreateShader(const std::string& vertexShader, const std::string& fragmentShader, const std::string& geometryShader)
{
    const bool bHasGeometryShader = geometryShader != "";
    GLCall(unsigned int program = glCreateProgram());
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    unsigned int gs = 0; 
    if(bHasGeometryShader)
    {
        gs = CompileShader(GL_GEOMETRY_SHADER, geometryShader);
        GLCall(glAttachShader(program, gs));
    }

    GLCall(glAttachShader(program, vs));
    GLCall(glAttachShader(program, fs));
    GLCall(glLinkProgram(program));
    GLCall(glValidateProgram(program));

    GLCall(glDeleteShader(vs));
    GLCall(glDeleteShader(fs));
    if(bHasGeometryShader)
    {
       GLCall(glDeleteShader(gs));
    }

    return program;
}

void Shader::StartTimer() {
    // Generate the query if it hasn't been created yet
    if (m_queryID == 0) {
        glGenQueries(1, &m_queryID);
    }
    
    // Start the timer
    glBeginQuery(GL_TIME_ELAPSED, m_queryID);
}

void Shader::StopTimer() {
    if(m_queryID != 0)
    {
          // End the timer query
        glEndQuery(GL_TIME_ELAPSED);

        m_elapsedTime = _GetElapsedTime();

        // Cleanup the query
        glDeleteQueries(1, &m_queryID);
        m_queryID = 0;
    }
}

void Shader::FindFileName()
{
    m_fileName = m_vertexFilePath;
    std::string fileName = std::filesystem::path(m_vertexFilePath).filename().string();

    size_t lastDotPos = fileName.find_last_of('.');
    if (lastDotPos != std::string::npos) {
        m_fileName = fileName.substr(0, lastDotPos);
    }
}

double Shader::_GetElapsedTime()
{
    // Check if the query result is available
    int available = 0;

    const int maxIterations = 10000000; // Max iterations to avoid an infinite loop
    int iterationCount = 0;

    while (!available && iterationCount < maxIterations) {
        glGetQueryObjectiv(m_queryID, GL_QUERY_RESULT_AVAILABLE, &available);
        iterationCount++;
    }

    if (available) {
        GLuint64 elapsed_time;
        glGetQueryObjectui64v(m_queryID, GL_QUERY_RESULT, &elapsed_time);

        // Return the elapsed time in milliseconds
        return elapsed_time / 1000000.0; // Convert from nanoseconds to milliseconds
    }

    // If the query is not available yet, return -1 or some indication
    return -1.0;
}

double Shader::GetElapsedTime() 
{
    return m_elapsedTime;
}

std::string Shader::GetName() const
{
    return m_fileName;
}
