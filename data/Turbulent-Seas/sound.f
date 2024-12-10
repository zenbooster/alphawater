

// Rough Seas, by Dave Hoskins.

// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
// By David Hoskins, 2022.

#define UI0 1597334673U
#define UI1 3812015801U
#define UI2 uvec2(UI0, UI1)
#define UIF (1.0 / float(0xffffffffU))

float fader(float edge0, float edge1, float x)
{
    float t = (x - edge0) / (edge1 - edge0);
    return  clamp(exp((t-.9825)*3.)-.0525, 0.0, 1.0);
}

vec2 hash21(uint q)
{
	uvec2 n = q * UI2;
	n = (n.x ^ n.y) * UI2;
	return vec2(n) * UIF;
}

vec2 hash21(float p)
{
	uvec2 n = uint(int(p)) * UI2;
	n = (n.x ^ n.y) * UI2;
	return vec2(n) * UIF;
}


vec2 noise2D(in float p)
{
	float f = fract(p);
    p = floor(p);
    f = f * f * (3.0 - 2.0 * f);
    vec2 res = mix(hash21(p), hash21(p + 1.0), f);
    return res-.5;
}


vec2 mainSound( int samp, float time )
{
    vec2 v, aud;
    float t= time;
    
    
    // Add vary volumes of different frequencies...
    v = noise2D(t*.6)*.5+.5;
    aud = noise2D(t*320.) * v;
    
    v = noise2D(t*.3)*.8+.2;
    aud += noise2D(t*800.)*v;

    v = noise2D(t*.3)*.8;
    aud += noise2D(t*1900.)*v;


    v = (noise2D(-t*.4)+noise2D(-t*.3))*.3;
    aud += noise2D(t*4400.)*v;

    v = (noise2D(t*.7) +noise2D(t*.22))*.2;
    aud += noise2D(t*10200.)*v;


    aud = clamp(aud, -1.0, 1.0) * fader(.0, 1.0,time) * fader(180.0, 170.0,time);
    
    return aud;
}