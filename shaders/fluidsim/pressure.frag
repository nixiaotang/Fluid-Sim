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
    
    // Boundary detection (walls)
    bool atLeft = gl_FragCoord.x < 1.5;
    bool atRight = gl_FragCoord.x > iResolution.x - 1.5;
    bool atBottom = gl_FragCoord.y < 1.5;
    bool atTop = gl_FragCoord.y > iResolution.y - 1.5;
    
    // Barrier detection for neighbors
    vec2 posL = x - vec2(texelSize.x, 0.0);
    vec2 posR = x + vec2(texelSize.x, 0.0);
    vec2 posT = x + vec2(0.0, texelSize.y);
    vec2 posB = x - vec2(0.0, texelSize.y);
    
    // Sample neighboring velocities for divergence calculation
    float uL = atLeft ? 0.0 : texture(uFluid, posL).x;
    float uR = atRight ? 0.0 : texture(uFluid, posR).x;
    float uT = atTop ? 0.0 : texture(uFluid, posT).y;
    float uB = atBottom ? 0.0 : texture(uFluid, posB).y;

    // Divergence: central difference with grid spacing dx=1
    // div = (uR - uL) / (2*dx) + (vT - vB) / (2*dy)
    float divergence = 0.5 * ((uR - uL) + (uT - uB));

    // Neumann BC: dp/dn = 0 at walls, so use center pressure at boundaries
    float pC = texture(pFluid, x).r;
    float pL = atLeft  ? pC : texture(pFluid, posL).r;
    float pR = atRight ? pC : texture(pFluid, posR).r;
    float pT = atTop   ? pC : texture(pFluid, posT).r;
    float pB = atBottom ? pC : texture(pFluid, posB).r;

    // Jacobi iteration: p = (pL + pR + pT + pB - dx² * div) / 4
    float pressure = (pR + pL + pT + pB - divergence) * 0.25;

    gl_FragColor = vec4(pressure, 0.0, 0.0, 1.0);
}
