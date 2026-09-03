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
uniform mat4 u_ModelView;

out vec2 v_TexCoord;
out vec4 v_Color;
out float v_FogDistance;

void main() {
    v_TexCoord = a_TexCoord;
    v_Color = a_Color;
    vec4 viewPos = u_ModelView * vec4(a_Position, 1.0);
    v_FogDistance = length(viewPos.xz);
    gl_Position = u_MVP * vec4(a_Position, 1.0);
}
)";

static const char* defaultTerrainFrag = R"(#version 300 es
precision highp float;

in vec2 v_TexCoord;
in vec4 v_Color;
in float v_FogDistance;

uniform sampler2D u_Texture;
uniform vec4 u_FogColor;
uniform float u_FogStart;
uniform float u_FogEnd;
uniform int u_FogEnabled;

out vec4 fragColor;

void main() {
    vec4 texColor = texture(u_Texture, v_TexCoord);
    float maxDiff = max(abs(texColor.r - texColor.g), max(abs(texColor.r - texColor.b), abs(texColor.g - texColor.b)));
    vec4 finalColor;
    if (maxDiff > 0.15) {
        float light = max(v_Color.r, max(v_Color.g, v_Color.b));
        finalColor = vec4(texColor.rgb * light, texColor.a * v_Color.a);
    } else {
        finalColor = texColor * v_Color;
    }

    if (finalColor.a < 0.1) {
        discard;
    }

    if (u_FogEnabled != 0 && u_FogEnd > u_FogStart) {
        float fogFactor = clamp((u_FogEnd - v_FogDistance) / (u_FogEnd - u_FogStart), 0.0, 1.0);
        finalColor.rgb = mix(u_FogColor.rgb, finalColor.rgb, fogFactor);
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

void TerrainShader::setupModelView(const Matrix4f& modelView)
{
    if (!inited) init();
    instance.bind();
    instance.setUniformMatrix4f("u_ModelView", modelView);
}

void TerrainShader::setupFog(bool enabled, float fogStart, float fogEnd, float r, float g, float b, float a)
{
    if (!inited) init();
    instance.bind();
    instance.setUniform1i("u_FogEnabled", enabled ? 1 : 0);
    instance.setUniform1f("u_FogStart", fogStart);
    instance.setUniform1f("u_FogEnd", fogEnd);
    instance.setUniform4f("u_FogColor", r, g, b, a);
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
