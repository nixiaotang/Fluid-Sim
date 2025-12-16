#version 330 core

precision highp float;
out vec4 FragColor;
#define gl_FragColor FragColor

uniform sampler2D uFluid;
uniform vec3 iResolution;
uniform int ColourType;

// HSV to RGB conversion (h in [0,1], s in [0,1], v in [0,1])
// Reference: https://gist.github.com/kylemcdonald/f8df3bc2f8d38ca2b7cb
vec3 hsv2rgb(float h, float s, float v) {
    vec3 c = vec3(h * 6.0, s, v);
    vec3 rgb = clamp(abs(mod(c.x + vec3(0.0, 4.0, 2.0), 6.0) - 3.0) - 1.0, 0.0, 1.0);
    return c.z * mix(vec3(1.0), rgb, c.y);
}

void main() {
    vec2 x = gl_FragCoord.xy / iResolution.xy;
    vec4 c = texelFetch(uFluid, ivec2(gl_FragCoord.xy), 0);
    float dye = c.z;

    vec3 colour;

    if (ColourType == 1) {
        float hue = c.w;

        float h = hue;
        float s = 1.0;
        float v = clamp(dye * 2.0, 0.0, 1.0);

        colour = hsv2rgb(h, s, v);

    } else {
        colour = vec3(dye, dye, dye);
    }
    
    gl_FragColor = vec4(colour, 1.0);
}