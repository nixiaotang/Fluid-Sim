#version 330 core
precision highp float;
out vec4 FragColor;
#define gl_FragColor FragColor

uniform vec3 iResolution;
uniform float iTime;
uniform vec2 iMouse;
uniform vec2 iMouseVel; // normalized velocity

uniform sampler2D uFluid;
uniform float DT;
uniform float InputRadius;
uniform float InputStrength;
uniform float DyeStrength;
uniform float VelocityDecay;
uniform float DyeDecay;
const float PI = 3.1415926539;

void main() {
    vec2 x = gl_FragCoord.xy / iResolution.xy;
    vec4 fluid = texture(uFluid, x);
    vec2 u = fluid.xy;
    float dye = fluid.z;
    float hue = fluid.w;

    vec2 dragMousePos = iMouse / iResolution.xy;

    if (length(iMouseVel) > 0.0) {
        float dist = length(x - dragMousePos);
        float gaussian = exp(-dist * dist / (InputRadius * InputRadius));

        vec2 force = iMouseVel * gaussian * InputStrength;
        u.xy += force * DT;
        
        dye += gaussian * DyeStrength * DT;

        // Determine new hue based on random colour setting
        float newHue;
        
        // Generate hue based on mouse direction + slow time cycle
        float dirHue = atan(iMouseVel.y, iMouseVel.x) / (2.0 * PI) + 0.5;
        float timeHue = fract(iTime * 0.00001);
        newHue = fract(dirHue + timeHue);
        
        // Blend new hue into existing, weighted by how much dye we're adding
        hue = mix(hue, newHue, clamp(gaussian, 0.0, 1.0));
    }

    u.xy *= (1.0 - VelocityDecay);
    dye *= (1.0 - DyeDecay);

    gl_FragColor = vec4(u, dye, hue);
}
