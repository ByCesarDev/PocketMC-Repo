#include "Shader.h"
#include "gles.h"
#include "../../platform/log.h"
#include <vector>

Shader::Shader()
:   m_programId(0)
{
}

Shader::~Shader()
{
    if (m_programId != 0) {
        glDeleteProgram(m_programId);
        m_programId = 0;
    }
}

unsigned int Shader::compileShader(unsigned int type, const std::string& source)
{
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, NULL);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        if (length > 0) {
            std::vector<char> message(length + 1);
            glGetShaderInfoLog(id, length, &length, &message[0]);
            LOGE("[Shader] Failed to compile %s shader:\n%s\n", 
                (type == GL_VERTEX_SHADER ? "vertex" : "fragment"), &message[0]);
        }
        glDeleteShader(id);
        return 0;
    }
    return id;
}

bool Shader::loadFromSource(const std::string& vertexSource, const std::string& fragmentSource)
{
#if defined(OPENGL_ES)
    return true;
#else
    unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexSource);
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

    if (vs == 0 || fs == 0) {
        if (vs != 0) glDeleteShader(vs);
        if (fs != 0) glDeleteShader(fs);
        return false;
    }

    m_programId = glCreateProgram();
    glAttachShader(m_programId, vs);
    glAttachShader(m_programId, fs);
    glLinkProgram(m_programId);

    int linkStatus;
    glGetProgramiv(m_programId, GL_LINK_STATUS, &linkStatus);
    if (linkStatus == GL_FALSE) {
        int length;
        glGetProgramiv(m_programId, GL_INFO_LOG_LENGTH, &length);
        if (length > 0) {
            std::vector<char> message(length + 1);
            glGetProgramInfoLog(m_programId, length, &length, &message[0]);
            LOGE("[Shader] Program linking failed:\n%s\n", &message[0]);
        }
        glDeleteProgram(m_programId);
        m_programId = 0;
        glDeleteShader(vs);
        glDeleteShader(fs);
        return false;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return true;
#endif
}

void Shader::bind() const
{
#if !defined(OPENGL_ES)
    if (m_programId != 0) {
        glUseProgram(m_programId);
    }
#endif
}

void Shader::unbind() const
{
#if !defined(OPENGL_ES)
    if (m_programId != 0) {
        glUseProgram(0);
    }
#endif
}

int Shader::getUniformLocation(const std::string& name)
{
    auto it = m_uniformLocations.find(name);
    if (it != m_uniformLocations.end()) {
        return it->second;
    }

    int location = glGetUniformLocation(m_programId, name.c_str());
    m_uniformLocations[name] = location;
    return location;
}

int Shader::getAttribLocation(const std::string& name)
{
    auto it = m_attribLocations.find(name);
    if (it != m_attribLocations.end()) {
        return it->second;
    }

    int location = glGetAttribLocation(m_programId, name.c_str());
    m_attribLocations[name] = location;
    return location;
}

void Shader::setUniformMatrix4f(const std::string& name, const Matrix4f& matrix)
{
    int loc = getUniformLocation(name);
    if (loc != -1) {
        glUniformMatrix4fv(loc, 1, GL_FALSE, matrix.getValues());
    }
}

void Shader::setUniform1i(const std::string& name, int value)
{
    int loc = getUniformLocation(name);
    if (loc != -1) {
        glUniform1i(loc, value);
    }
}

void Shader::setUniform1f(const std::string& name, float value)
{
    int loc = getUniformLocation(name);
    if (loc != -1) {
        glUniform1f(loc, value);
    }
}

void Shader::setUniform2f(const std::string& name, float x, float y)
{
    int loc = getUniformLocation(name);
    if (loc != -1) {
        glUniform2f(loc, x, y);
    }
}

void Shader::setUniform3f(const std::string& name, float x, float y, float z)
{
    int loc = getUniformLocation(name);
    if (loc != -1) {
        glUniform3f(loc, x, y, z);
    }
}

void Shader::setUniform4f(const std::string& name, float x, float y, float z, float w)
{
    int loc = getUniformLocation(name);
    if (loc != -1) {
        glUniform4f(loc, x, y, z, w);
    }
}
