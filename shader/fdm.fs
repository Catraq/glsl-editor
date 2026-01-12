#version 330 core

uniform vec2 resolution;
uniform vec2 mouse;
uniform float time;

layout (location=0) out vec4 fragcolor;


float random (vec2 uv) {
    return fract(sin(dot(uv.xy, vec2(8.91313,1.233))));
}

float noise (vec2 uv) {
    vec2 i = floor(uv);
    vec2 f = fract(uv);

    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    vec2 u = f * f * f;

    return mix(a, b, u.x) + (c - a)* u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}


float fbm (vec2 uv) {
    float v = 0.0;
    float a = 0.5;
    for (int i = 0; i < 4; ++i) {
        v += a * noise(uv);
        uv = uv * 2.13123123;
        a *= 0.5;
    }
    return v;
}

void main() {
    vec2 uv = gl_FragCoord.xy/resolution.xy*3.;
    vec3 color = vec3(1.0);
    float t = fbm(uv*0.123497872);
    float h = fbm(t*1.76623 + uv*2.123497872);
    float f = fbm(uv*1.99323*t+h);


    fragcolor = vec4(f*color,1.0);
}

