#include "GuiShader.h"
#include "../../platform/log.h"

Shader GuiShader::instance;
bool GuiShader::inited = false;

static const char* defaultGuiVert = R"(#version 300 es
precision highp float;

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec4 a_Color;

uniform mat4 u_OrthoMatrix;

out vec2 v_TexCoord;
out vec4 v_Color;

void main() {
    v_TexCoord = a_TexCoord;
    v_Color = a_Color;
    gl_Position = u_OrthoMatrix * vec4(a_Position, 1.0);
}
)";

static const char* defaultGuiFrag = R"(#version 300 es
precision highp float;

in vec2 v_TexCoord;
in vec4 v_Color;

uniform sampler2D u_Texture;
uniform bool u_UseTexture;

out vec4 fragColor;

void main() {
    vec4 texColor = u_UseTexture ? texture(u_Texture, v_TexCoord) : vec4(1.0);
    vec4 finalColor = texColor * v_Color;

    if (finalColor.a < 0.01) {
        discard;
    }

    fragColor = finalColor;
}
)";

bool GuiShader::init()
{
    if (inited) return true;
    inited = instance.loadFromSource(defaultGuiVert, defaultGuiFrag);
    if (!inited) {
        LOGE("Failed to load embedded GUI shaders!\n");
    } else {
        LOGI("GuiShader initialized successfully.\n");
    }
    return inited;
}

void GuiShader::setupOrtho(float width, float height)
{
    if (!inited) init();
    Matrix4f ortho = Matrix4f::createOrtho(0.0f, width, height, 0.0f, 1000.0f, 3000.0f);
    Matrix4f trans = Matrix4f::createTranslation(0.0f, 0.0f, -2000.0f);
    Matrix4f mvp = ortho * trans;

    instance.bind();
    instance.setUniformMatrix4f("u_OrthoMatrix", mvp);
    instance.setUniform1i("u_Texture", 0);
    instance.setUniform1i("u_UseTexture", 1);
}

void GuiShader::bind()
{
    if (!inited) init();
    instance.bind();
}

void GuiShader::unbind()
{
    instance.unbind();
}
