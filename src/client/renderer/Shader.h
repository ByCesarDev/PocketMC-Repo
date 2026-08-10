#ifndef NET_MINECRAFT_CLIENT_RENDERER__Shader_H__
#define NET_MINECRAFT_CLIENT_RENDERER__Shader_H__

#include <string>
#include <unordered_map>
#include "../../util/Matrix4f.h"

class Shader
{
public:
    Shader();
    ~Shader();

    bool loadFromSource(const std::string& vertexSource, const std::string& fragmentSource);
    void bind() const;
    void unbind() const;

    int getUniformLocation(const std::string& name);
    int getAttribLocation(const std::string& name);

    void setUniformMatrix4f(const std::string& name, const Matrix4f& matrix);
    void setUniform1i(const std::string& name, int value);
    void setUniform1f(const std::string& name, float value);
    void setUniform2f(const std::string& name, float x, float y);
    void setUniform3f(const std::string& name, float x, float y, float z);
    void setUniform4f(const std::string& name, float x, float y, float z, float w);

    unsigned int getProgramId() const { return m_programId; }

private:
    unsigned int compileShader(unsigned int type, const std::string& source);

    unsigned int m_programId;
    std::unordered_map<std::string, int> m_uniformLocations;
    std::unordered_map<std::string, int> m_attribLocations;
};

#endif /* NET_MINECRAFT_CLIENT_RENDERER__Shader_H__ */
