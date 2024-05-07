
#define R iResolution
#define T(uv) texture(iChannel0, uv).r
float gyroid (vec3 seed) { return dot(sin(seed),cos(seed.yzx)); }
float fbm (vec3 seed) {
    float result = 0.;
    float a = .5;
    for (int i = 0; i < 3; ++i) {
        result += gyroid(seed/a)*a;
        a /= 3.;
    }
    return result;
}

// Dave Hoskins https://www.shadertoy.com/view/4djSRW
vec2 hash21(float p)
{
	vec3 p3 = fract(vec3(p) * vec3(.1031, .1030, .0973));
	p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.xx+p3.yz)*p3.zy);
}

// Victor Shepardson + Inigo Quilez 
// https://www.shadertoy.com/view/XlXcW4
const uint k = 1103515245U;  // GLIB C
vec3 hash( uvec3 x )
{
    x = ((x>>8U)^x.yzx)*k;
    x = ((x>>8U)^x.yzx)*k;
    x = ((x>>8U)^x.yzx)*k;
    return vec3(x)*(1.0/float(0xffffffffU));
}