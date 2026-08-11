#ifndef NET_MINECRAFT_CLIENT_RENDERER__EntityShader_H__
#define NET_MINECRAFT_CLIENT_RENDERER__EntityShader_H__

#include "Shader.h"
#include "../../util/Matrix4f.h"

class EntityShader
{
public:
    static Shader instance;
    static bool inited;

    static bool init();
    static void setupMVP(const Matrix4f& mvp);
    static void setOverlayColor(float r, float g, float b, float a);
    static void bind();
    static void unbind();
};

#endif /* NET_MINECRAFT_CLIENT_RENDERER__EntityShader_H__ */
