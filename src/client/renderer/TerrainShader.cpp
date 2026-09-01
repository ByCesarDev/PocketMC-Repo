#include "TerrainShader.h"
#include "../../platform/log.h"

Shader TerrainShader::instance;
bool TerrainShader::inited = false;

static const char* defaultTerrainVert = R"(#version 300 es
precision highp float;

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec4 a_Color;

uniform mat4 u_MVP;

out vec2 v_TexCoord;
out vec4 v_Color;

void main() {
    v_TexCoord = a_TexCoord;
    v_Color = a_Color;
    gl_Position = u_MVP * vec4(a_Position, 1.0);
}
)";

static const char* defaultTerrainFrag = R"(#version 300 es
precision highp float;

in vec2 v_TexCoord;
in vec4 v_Color;

uniform sampler2D u_Texture;

out vec4 fragColor;

void main() {
    vec4 texColor = texture(u_Texture, v_TexCoord);
    vec4 finalColor = texColor * v_Color;

    if (finalColor.a < 0.1) {
        discard;
    }

    fragColor = finalColor;
}
)";

bool TerrainShader::init()
{
    if (inited) return true;
    inited = instance.loadFromSource(defaultTerrainVert, defaultTerrainFrag);
    if (!inited) {
        LOGE("Failed to load embedded Terrain shaders!\n");
    }
    return inited;
}

void TerrainShader::setupMVP(const Matrix4f& mvp)
{
    if (!inited) init();
    instance.bind();
    instance.setUniformMatrix4f("u_MVP", mvp);
    instance.setUniform1i("u_Texture", 0);
}

void TerrainShader::bind()
{
    if (!inited) init();
    instance.bind();
}

void TerrainShader::unbind()
{
    instance.unbind();
}
