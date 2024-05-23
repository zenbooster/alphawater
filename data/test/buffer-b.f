highp float rand(vec2 co)
{
    highp float a = 12.9898;
    highp float b = 78.233;
    highp float c = 43758.5453;
    highp float dt= dot(co.xy ,vec2(a,b));
    highp float sn= mod(dt,3.14);
    return fract(sin(sn) * c);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 uv = fragCoord/iResolution.xy;
    float dx = 1. / iResolution.x;
    float dy = 1. / iResolution.y;
    vec3 c;
    
    c = texture(iChannel0, uv).rgb;
    float rx = rand(uv) - 0.5;
    float ry = rand(uv.yx) - 0.5;

    float fade = 1.;
    float dist = 0.1;
    c = texture(iChannel0, uv+vec2(rx, ry) * dist).rgb * (1. - fade / 255.);

    fragColor = vec4(c, 1.);
}