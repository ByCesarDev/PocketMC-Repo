#include "EntityShader.h"
#include "../../platform/log.h"

Shader EntityShader::instance;
bool EntityShader::inited = false;

static const char* defaultEntityVert = R"(#version 300 es
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

static const char* defaultEntityFrag = R"(#version 300 es
precision highp float;

in vec2 v_TexCoord;
in vec4 v_Color;

uniform sampler2D u_Texture;
uniform vec4 u_OverlayColor;

out vec4 fragColor;

void main() {
    vec4 texColor = texture(u_Texture, v_TexCoord);
    vec4 finalColor = texColor * v_Color + vec4(u_OverlayColor.rgb * u_OverlayColor.a, 0.0);

    if (texColor.a < 0.1) {
        discard;
    }

    fragColor = finalColor;
}
)";

bool EntityShader::init()
{
    if (inited) return true;
    inited = instance.loadFromSource(defaultEntityVert, defaultEntityFrag);
    if (!inited) {
        LOGE("Failed to load embedded Entity shaders!\n");
    } else {
        LOGI("EntityShader initialized successfully.\n");
    }
    return inited;
}

void EntityShader::setupMVP(const Matrix4f& mvp)
{
    if (!inited) init();
    instance.bind();
    instance.setUniformMatrix4f("u_MVP", mvp);
    instance.setUniform1i("u_Texture", 0);
    instance.setUniform4f("u_OverlayColor", 0.0f, 0.0f, 0.0f, 0.0f);
}

void EntityShader::setOverlayColor(float r, float g, float b, float a)
{
    if (!inited) init();
    instance.setUniform4f("u_OverlayColor", r, g, b, a);
}

void EntityShader::bind()
{
    if (!inited) init();
    instance.bind();
}

void EntityShader::unbind()
{
    instance.unbind();
}
