float sdfCircle(vec2 uv, float r, vec2 offset) {
    float x = uv.x - offset.x;
    float y = uv.y - offset.y;

    float d = length(vec2(x, y)) - r;

    return step(0., -d);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 uv = fragCoord/iResolution.xy;
    vec2 muv = uv - 0.5;
    muv.x *= iResolution.x/iResolution.y; // fix aspect ratio
    vec2 offset = vec2(0);
    vec3 c = vec3(0);
    float circle = sdfCircle(muv, 0.1, offset);

    float half_cycle = 0.5;
    float v = mod(iTime, 2. * half_cycle);
    int i_half_cycle = int(half_cycle);

    float exposition = half_cycle / 10.;
    if((v >= 0. && v < exposition) || (v >= half_cycle && v < half_cycle + exposition))
    {
        vec3 sc = mod(iTime, (2. * half_cycle)) < half_cycle ? vec3(1, 1, 0) : vec3(1, 0, 0);
        c = mix(c, sc, circle);
    }

    float dx = 1. / iResolution.x;
    float dy = 1. / iResolution.y;

    float k = 0.6;
    vec3 c0 = texture(iChannel0, uv).rgb * (1. - k);
    c0 += c * k;

    c0 += texture(iChannel0, uv - vec2(-dx, 0)).rgb;
    c0 += texture(iChannel0, uv - vec2(dx, 0)).rgb;
    c0 += texture(iChannel0, uv - vec2(0., -dy)).rgb;
    c0 += texture(iChannel0, uv - vec2(0., dy)).rgb;

    c0 /= 4.5;

    fragColor = vec4(c0, 1.0);
}