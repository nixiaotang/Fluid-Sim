#version 330 core

precision highp float;
out vec4 FragColor;
#define gl_FragColor FragColor

uniform vec3 iResolution;
uniform sampler2D uFluid;
uniform sampler2D pFluid;
uniform float DT;

void main() {
    vec2 x = gl_FragCoord.xy / iResolution.xy;
    vec2 texelSize = 1.0 / iResolution.xy;
    vec4 fluid = texture(uFluid, x);
    vec2 u = fluid.xy;
    float dye = fluid.z;
    
    // Boundary detection (walls)
    bool atLeft = gl_FragCoord.x < 1.5;
    bool atRight = gl_FragCoord.x > iResolution.x - 1.5;
    bool atBottom = gl_FragCoord.y < 1.5;
    bool atTop = gl_FragCoord.y > iResolution.y - 1.5;
    
    // Compute pressure gradient: ∇p
    vec2 posL = x - vec2(texelSize.x, 0.0);
    vec2 posR = x + vec2(texelSize.x, 0.0);
    vec2 posT = x + vec2(0.0, texelSize.y);
    vec2 posB = x - vec2(0.0, texelSize.y);

    float pL = texture(pFluid, posL).x;
    float pR = texture(pFluid, posR).x;
    float pB = texture(pFluid, posB).x;
    float pT = texture(pFluid, posT).x;
    
    vec2 gradP = vec2(pR - pL, pT - pB) * 0.5;

    // Project velocity to be divergence-free
    u -= gradP;
    
    // Enforce no-penetration BC: zero normal velocity at walls
    if (atLeft || atRight) u.x = 0.0;
    if (atBottom || atTop) u.y = 0.0;

    gl_FragColor = vec4(u, dye, fluid.w);

}