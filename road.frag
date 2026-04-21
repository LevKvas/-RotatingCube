#version 330 core
in vec2 vTex;

// textures
uniform sampler2D texBase;    // stone
uniform sampler2D texMarkup;  // marking
uniform sampler2D texCracks;  // cracks

uniform bool multiTexture;    // draw one or mix

out vec4 fragColor;

void main() {
    if (!multiTexture) {
        // one texture
        fragColor = texture(texBase, vTex);
    } else {
        // mixing
        vec4 base = texture(texBase, vTex * 5.0); // smaller stone
        vec4 markup = texture(texMarkup, vTex);
        vec4 cracks = texture(texCracks, vTex * 2.5);

        // mix
        vec4 res = mix(base, markup, markup.r);

        // muptiply
        fragColor = res * cracks;
    }
}
