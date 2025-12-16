#version 330 core

precision highp float;
out vec4 FragColor;
#define gl_FragColor FragColor

uniform vec3 iResolution;
uniform sampler2D uFluid;
uniform float DT;

void main() {
    vec2 x = gl_FragCoord.xy / iResolution.xy;
    vec2 u = texture(uFluid, x).xy;

    // Trace back to previous position
    vec2 prev_x = x - (u / iResolution.xy) * DT;
    vec4 prev_u = texture(uFluid, prev_x);

    gl_FragColor = prev_u;
}
