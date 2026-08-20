#version 300 es
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
