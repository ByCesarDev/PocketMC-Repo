#ifndef NET_MINECRAFT_CLIENT_RENDERER__GuiShader_H__
#define NET_MINECRAFT_CLIENT_RENDERER__GuiShader_H__

#include "Shader.h"
#include "../../util/Matrix4f.h"

class GuiShader
{
public:
    static Shader instance;
    static bool inited;

    static bool init();
    static void setupOrtho(float width, float height);
    static void bind();
    static void unbind();
};

#endif /* NET_MINECRAFT_CLIENT_RENDERER__GuiShader_H__ */
