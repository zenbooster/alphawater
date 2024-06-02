// Find t so that mix(a,b,t) = x
float unmix(float a, float b, float x) {
    return (x - a)/(b - a);
}

float rand(in vec2 p) {
    return fract(sin(dot(p,vec2(12.9898,78.233))) * 43758.5453);
}

float noise(in vec2 p) {
    vec2 pi = floor(p);
    vec2 pf = fract(p);

    float r00 = rand(vec2(pi.x    ,pi.y    ));
    float r10 = rand(vec2(pi.x+1.0,pi.y    ));
    float r01 = rand(vec2(pi.x    ,pi.y+1.0));
    float r11 = rand(vec2(pi.x+1.0,pi.y+1.0));

    return mix(mix(r00, r10, pf.x), mix(r01, r11, pf.x), pf.y);
}

float fbm(vec2 uv) {
    vec2 p = uv*256.0;
    
    float v = noise(p);

    p *= 0.3;
    v = mix(v, noise(p), 0.8);

    p *= 0.3;
    v = mix(v, noise(p), 0.8);

    p *= 0.3;
    v = mix(v, noise(p), 0.8);

    return v;
}

float nearInt(float x) {
    return pow(0.5 + 0.5*cos(x*6.28),10.0);
}

float fbmWithBorder(vec2 uv) {
    float r = fbm(uv);
    return mix(1.0, fbm(uv), (1.0-nearInt(uv.x))*(1.0-nearInt(uv.y)) );
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {

    vec2 uv = fragCoord/iResolution.xy;

    float heightTerrain = fbmWithBorder(uv);
    float cloudIntensity = heightTerrain;
    
    float starVal = noise(uv*1024.0);
    float starIntensity = unmix(0.95, 1.0, starVal) * step(0.95, starVal);

    fragColor = vec4(heightTerrain, cloudIntensity, starIntensity, 0.0);
}
