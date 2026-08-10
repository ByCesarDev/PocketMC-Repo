#version 300 es
precision highp float;

in vec2 v_TexCoord;
in vec4 v_Color;
in float v_FogDepth;

uniform sampler2D u_Texture;
uniform bool u_UseTexture;
uniform bool u_UseFog;
uniform vec4 u_FogColor;
uniform float u_FogStart;
uniform float u_FogEnd;

out vec4 fragColor;

void main() {
    vec4 texColor = u_UseTexture ? texture(u_Texture, v_TexCoord) : vec4(1.0);
    vec4 finalColor = texColor * v_Color;

    if (finalColor.a < 0.1) {
        discard;
    }

    if (u_UseFog && u_FogEnd > u_FogStart) {
        float fogFactor = clamp((u_FogEnd - v_FogDepth) / (u_FogEnd - u_FogStart), 0.0, 1.0);
        finalColor.rgb = mix(u_FogColor.rgb, finalColor.rgb, fogFactor);
    }

    fragColor = finalColor;
}
