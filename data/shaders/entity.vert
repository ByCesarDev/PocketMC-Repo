#version 300 es
precision highp float;

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec4 a_Color;
layout(location = 3) in vec3 a_Normal;

uniform mat4 u_MVP;
uniform mat4 u_ModelView;
uniform vec4 u_LightColor;

out vec2 v_TexCoord;
out vec4 v_Color;
out float v_FogDepth;

void main() {
    v_TexCoord = a_TexCoord;
    v_Color = a_Color * u_LightColor;
    
    vec4 viewPos = u_ModelView * vec4(a_Position, 1.0);
    v_FogDepth = length(viewPos.xyz);
    
    gl_Position = u_MVP * vec4(a_Position, 1.0);
}
