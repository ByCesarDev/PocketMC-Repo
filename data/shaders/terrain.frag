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

    vec4 texColor = u_UseTexture ? texture(u_Texture, v_TexCoord) : vec4(1.0);
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

    if (u_UseFog && u_FogEnd > u_FogStart) {
        float fogFactor = clamp((u_FogEnd - v_FogDepth) / (u_FogEnd - u_FogStart), 0.0, 1.0);
        finalColor.rgb = mix(u_FogColor.rgb, finalColor.rgb, fogFactor);
    }

    fragColor = finalColor;
}
