#ifndef NET_MINECRAFT_CLIENT_RENDERER__TerrainShader_H__
#define NET_MINECRAFT_CLIENT_RENDERER__TerrainShader_H__

#include "Shader.h"
#include "../../util/Matrix4f.h"

class TerrainShader
{
public:
    static Shader instance;
    static bool inited;

    static bool init();
    static void setupMVP(const Matrix4f& mvp);
    static void bind();
    static void unbind();
};

#endif /* NET_MINECRAFT_CLIENT_RENDERER__TerrainShader_H__ */
