
// My first complete scene using raymarching.

// The lighthouse is a composition using simple primitives.
// The ocean is made with a stack of sine waves and some noise to emulate wind.

// Barely any optimizations so performance is pretty bad.
// The code could also use a clean-up too.



const float MATERIAL_SKY = 0.0;
const float MATERIAL_SEA = 1.0;
const float MATERIAL_ROCK = 10.0;
const float MATERIAL_LIGHTHOUSE = 100.0;
const float MATERIAL_LIGHTHOUSE_WINDOW_LEDGE = 101.0;
const float MATERIAL_LIGHTHOUSE_WINDOW = 102.0;
const float MATERIAL_LIGHTHOUSE_BEAM = 103.0;

const float MAX_DIST = 700.0;

const vec3 lighthouseOrigin = vec3(-40.0, 16.0, 220.0);
const vec3 lighthouseLantern = lighthouseOrigin + vec3(0.0, 25.0, 0.0);



float odd(float v) {
	
	return ceil(fract(v * 0.5));

}


// https://iquilezles.org/articles/distfunctions/
float smin( float a, float b, float k)
{

  float h = max(k - abs(a - b), 0.0) / k;
  return min(a, b) - h * h * h * k * (1.0/6.0);

}


mat2 rotationTriple(float a, float b, float c)
{
    return mat2(a/c, b/c, -b/c, a/c);
}


// https://iquilezles.org/articles/distfunctions/
float opSubtraction( float d1, float d2 )
{
    return max(-d1,d2);
}


// https://iquilezles.org/articles/distfunctions/
float opSmoothSubtraction( float d1, float d2, float k )
{
    float h = clamp( 0.5 - 0.5*(d2+d1)/k, 0.0, 1.0 );
    return mix( d2, -d1, h ) + k*h*(1.0-h);
}



// PRIMITIVES
// https://iquilezles.org/articles/distfunctions/


float dot2(vec2 v)
{

    return v.x * v.x + v.y * v.y;

}


float sdOrientedBox( in vec2 p, in vec2 a, in vec2 b, float th )
{
    float l = length(b-a);
    vec2  d = (b-a)/l;
    vec2  q = (p-(a+b)*0.5);
          q = mat2(d.x,-d.y,d.y,d.x)*q;
          q = abs(q)-vec2(l,th)*0.5;
    return length(max(q,0.0)) + min(max(q.x,q.y),0.0);    
}


float sdTunnel( in vec2 p, in vec2 wh )
{
    p.x = abs(p.x); p.y = -p.y;
    vec2 q = p - wh;

    float d1 = dot2(vec2(max(q.x,0.0),q.y));
    q.x = (p.y>0.0) ? q.x : length(p)-wh.x;
    float d2 = dot2(vec2(q.x,max(q.y,0.0)));
    float d = sqrt( min(d1,d2) );
    
    return (max(q.x,q.y)<0.0) ? -d : d;
}


float sdCircle( vec2 p, float r )
{
    return length(p) - r;
}


float sdSphere( vec3 p, float s )
{
    return length(p)-s;
}


float sdBox( vec3 p, vec3 b )
{
    vec3 d = abs(p) - b;
    return min(max(d.x,max(d.y,d.z)),0.0) + length(max(d,0.0));
}


// vertical
float sdCylinder( vec3 p, vec2 h )
{
    vec2 d = abs(vec2(length(p.xz),p.y)) - h;
    return min(max(d.x,d.y),0.0) + length(max(d,0.0));
}


float sdCylinderInfinite( vec3 p, vec3 c )
{
  return length(p.xz-c.xy)-c.z;
}


float sdVerticalCapsule( vec3 p, float h, float r )
{
  p.y -= clamp( p.y, 0.0, h );
  return length( p ) - r;
}


float sdCapsule( vec3 p, vec3 a, vec3 b, float r )
{
  vec3 pa = p - a, ba = b - a;
  float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
  return length( pa - ba*h ) - r;
}


float sdTorus( vec3 p, vec2 t )
{
  vec2 q = vec2(length(p.xz)-t.x,p.y);
  return length(q)-t.y;
}


// UTILITY FUNCTIONS


vec2 pickClosest( vec2 A, vec2 B )
{

    return (A.x < B.x) ? A : B;

}



vec2 pickFarthest( vec2 A, vec2 B )
{

    return (A.x > B.x) ? A : B;

}


float random(vec2 p) {

  return fract(sin(dot(p, vec2(101.2, 973.0))) * 43758.5453123);

}


float randomSeed(float i, float s) {

	return fract(fract(sin(i)) * s);

}


vec2 random2D(vec2 i) {
	
	float d = dot(i, vec2(22.9876, 34.4388));

	return vec2( randomSeed( d, 3141592.65359 ), randomSeed( d, 2951414.95356 ) ) * 2.0 - 1.0;

}


float noise2D(vec2 p) {

  vec2 i = floor(p);
  vec2 f = fract(p);

  float a = random(i);
  float b = random(i + vec2(1, 0));
  float c = random(i + vec2(0, 1));
  float d = random(i + vec2(1, 1));

  vec2 u = vec2(f * f * (3.0 - 2.0 * f));
  
  float ab = a + (b - a) * u.x;
  float cd = c + (d - c) * u.x;
  float ab_cd = ab + (cd - ab) * u.y;

  return ab_cd;

}


float gradNoise2D(vec2 p) {

    vec2 f = fract(p);

	vec2 fl = mod(floor(p), 20.0);
	vec2 ce = mod(ceil(p), 20.0);

	vec2 a = random2D(vec2(fl.x, fl.y));
	vec2 b = random2D(vec2(ce.x, fl.y));
	vec2 c = random2D(vec2(fl.x, ce.y));
	vec2 d = random2D(vec2(ce.x, ce.y));

	float da = dot(a, f - vec2(0.0, 0.0));
	float db = dot(b, f - vec2(1.0, 0.0));
	float dc = dot(c, f - vec2(0.0, 1.0));
	float dd = dot(d, f - vec2(1.0, 1.0));
	
	f = smoothstep(0.0, 1.0, f);
	
	return 0.5 + mix( mix(da, db, f.x), mix(dc, dd, f.x), f.y );

}


float GradientNoise2D(vec2 p, int i, float d) {
    
    float f = 2.0;
    float w = 0.5;
    
    float n = gradNoise2D(p) * w;
    mat2 r = mat2(4.0/5.0, 3.0/5.0, -3.0/5.0, 4.0/5.0);
    
    for (int j=0; j<i; j++) {
    
        p = p * r;
        w = w * d;
        
        n += gradNoise2D(p * f) * w;
    
        f = f * 2.0;
    
    }
    
    return n;

}


float sineB(vec2 rp, float time) {

  // float a = sin((rp.x * 0.117351 + rp.y * 0.917892) + time);
  // float b = sin((rp.x * 0.291752 + rp.y * 0.123639) + time);
  
  float a = sin((rp.x * 0.117351 + rp.y * 0.917892) + time);
  float b = sin((rp.x * -0.991752 + rp.y * 0.123639) + time);

  return mix(a, b, 0.6667);

}


float sineA(vec2 q, float time) {
  
  float o = sin(q.y + time);
  
  float s = sin(q.x + time);
  
  s += sin(2.0 * q.x + 0.4971 * o) * 0.5;
  s += sin(4.0 * q.x + 1.1238 * o) * 0.25;
  s += sin(8.0 * q.x + 2.5781 * o) * 0.125;

  return s;

}



float paintedLetters( vec2 p )
{
    
    float vth = 0.15;
    float hth = vth * 0.85;

    // A
    p = p + vec2(2.35, 0.0);
    vec2 w = vec2(0.8, 1.0);
    float d1 = sdOrientedBox( p - vec2(0.5 * w.x, 1.0), vec2(0.0, -0.47) * w, vec2(0.49, 0.48) * w, hth * 1.02 );
    float d2 = sdOrientedBox( p - vec2(0.5 * w.x, 1.0), vec2(1.0, -0.47) * w, vec2(0.51, 0.48) * w, vth * 0.92 );
    float h1 = sdOrientedBox( p - vec2(0.0, 0.35), vec2(0.65, 0.5) * w, vec2(1.35, 0.5) * w, hth * 0.85 );
    float ap = sdOrientedBox( p - vec2(0.5 * w.x, 1.48), vec2(0.5, -hth * 0.2) * w, vec2(0.5, hth * 0.2) * w, hth );
    float t1 = sdOrientedBox( p - vec2(0.0 * w.x, 0.53), vec2(0.5, -hth * 0.2) * w, vec2(0.5, hth * 0.175) * w, hth );
    float t2 = sdOrientedBox( p - vec2(1.0 * w.x, 0.53), vec2(0.5, -hth * 0.2) * w, vec2(0.5, hth * 0.2) * w, hth );
    
    float A = min(min(ap, min(t1, t2)), min(h1, min(d1, d2)));

    // R
    p = p + vec2(-1.35, 0.0);
    w = vec2(0.8, 1.0);
    float v1 = sdOrientedBox( p - vec2(vth * 0.5, 0.5), vec2(0.0, 0.0) * w, vec2(0.0, 1.0) * w, vth );
    float a1 = abs(sdTunnel( p.yx - vec2(1.196, 0.6 - vth), vec2(0.24, 0.35))) - hth * 0.5;
    d1 = sdOrientedBox( p - vec2(0.397 + vth * 0.5, 0.955), vec2(0.0, 0.01) * w, vec2(0.45 - hth, -0.414) * w, hth * 1.07 );
    t1 = sdOrientedBox( p - vec2(0.41 * w.x, 0.53), vec2(0.5, -hth * 0.2) * w, vec2(0.5, hth * 0.35) * w, hth * 0.95 );
    float R = min(v1, min(a1, min(d1, t1)));
    
    // hyphen
    p = p + vec2(-1.1, 0.0);
    w = vec2(0.8, 1.0);
    float hyphen = sdOrientedBox( p - vec2(0.0, 0.95), vec2(-0.4, 0.0) * w, vec2(0.4, 0.0) * w, hth * 0.95 );    

    // M
    p = p + vec2(-0.43, 0.0);
    w = vec2(0.8, 1.0);
    v1 = sdOrientedBox( p - vec2(vth * 0.5, 0.5), vec2(0.0, 0.0) * w, vec2(0.0, 1.0) * w, vth );
    float v2 = sdOrientedBox( p - vec2(1.0 * w.x - vth * 0.5, 0.5), vec2(0.0, 0.0) * w, vec2(0.0, 1.0) * w, vth );
    
    d1 = sdOrientedBox( p - vec2(vth * 0.6, 1.461), vec2(0.01, 0.0) * w, vec2(0.5 - hth, -0.5) * w, hth * 0.98 );
    d2 = sdOrientedBox( p - vec2(1.0 * w.x - vth * 0.6, 1.461), vec2(-0.5 + hth, -0.5) * w, vec2(-0.01, 0.0) * w, hth * 0.98 );
    ap = sdOrientedBox( p - vec2(0.5 * w.x, 0.99), vec2(0.0, -hth * 0.5) * w, vec2(0.0, hth * 0.5) * w, hth );
    float M = min(v1, min(v2, min(ap, min(d1, d2))));

    // E
    p = p + vec2(-0.97, 0.0);
    w = vec2(0.65, 1.0);
    float v = sdOrientedBox( p - vec2(vth * 0.5, 0.5), vec2(0.0, 0.0) * w, vec2(0.0, 1.0) * w, vth );
    
    h1 = sdOrientedBox( p - vec2(0.0, hth * 0.5), vec2(0.0, 0.5) * w, vec2(1.0, 0.5) * w, hth );
    float h2 = sdOrientedBox( p - vec2(0.0, 0.5), vec2(0.0, 0.5) * w, vec2(0.8, 0.5) * w, hth );
    float h3 = sdOrientedBox( p - vec2(0.0, 1.0 - hth * 0.5), vec2(0.0, 0.5) * w, vec2(1.0, 0.5) * w, hth );
    float E = min(v, min(h1, min(h2, h3)));
    
    // N
    p = p + vec2(-0.78, 0.0);
    w = vec2(0.7, 1.0);
    v1 = sdOrientedBox( p - vec2(vth * 0.5, 0.5), vec2(0.0, 0.0) * w, vec2(0.0, 1.0) * w, vth );
    v2 = sdOrientedBox( p - vec2(1.0 * w.x - vth * 0.5, 0.5), vec2(0.0, 0.0) * w, vec2(0.0, 1.0) * w, vth );
    d1 = sdOrientedBox( p - vec2(0.5 * w.x, 1.0), vec2(-0.5 + hth * 1.1, 0.5 - hth * 0.25) * w, vec2(0.5 - hth * 1.1, -0.5 + hth * 0.25) * w, hth * 1.05 );
    
    float N = min(v1, min(v2, d1));
    
    return min(A, min(R, min(hyphen, min(M, min(E, N)))));

}


const mat2 mr0 = mat2(0.7071, 0.7071, -0.7071, 0.7071);
const mat2 mr1 = mat2(0, 1, -1, 0);
const mat2 mr2 = mat2(7.0/24.0, 24.0/25.0, -24.0/25.0, 7.0/25.0);
const mat2 mr3 = mat2(48.0/73.0, 55.0/73.0, -55.0/73.0, 48.0/73.0);


float waterMotion(vec3 rp, float time) {

  if (rp.y > 10.0) { return rp.y; }

  // level oscillation
  float s = 40.0 + (sin(time * 0.01) * 10.0);
  
  vec2 q = rp.xz * 0.002;
  
  vec2 q0 = mr0 * q;
  float a = sineA(q0, time * 0.8783);

  vec2 q1 = mr1 * q;
  float b = sineA(q1, time * 0.5631);

  // large waves
  s += a * 30.0 + b * 30.0;
  
  // medium waves
  float c = sineB(mr2 * q0 * 8.0, time * 0.93);
  c += sineB(mr3 * q1 * 16.0, time * 2.39) * 0.25;
  
  s += (c * 6.0);
  
  // lighthouse swell
  
  float dist = length(rp.xz / 29.0 - lighthouseOrigin.xz);
  dist = 1.0 - smoothstep(10.0, 40.0, dist);
  
  s += dist * (10.0 + a * 10.0 + b * 10.0 + c * 5.0) * (3.0 + sin(iTime));
  
  // wind noise
  
  float n = GradientNoise2D( vec2(rp.x + iTime * 2.5, rp.z) * 0.011, 5, 0.4 );

  float t = smoothstep(1000.0, 30000.0, rp.z);
  
  s -= (1.0 - t) * n * (24.0 + dist * 24.0);

  return s * 0.75;

}


// MATERIAL FUNCTIONS


float brickValueNoise2D(vec2 p, float width) {

  	vec2 q = floor(p);
  	float xo = odd(q.y);	
  	q.x = floor(q.x / width - (xo * 0.5));

    vec2 f = fract(q);

	vec2 fl = floor(q);
	vec2 ce = ceil(q);

	float a = random(vec2(fl.x, fl.y));
	float b = random(vec2(ce.x, fl.y));
	float c = random(vec2(fl.x, ce.y));
	float d = random(vec2(ce.x, ce.y));
	
	return 0.25 + mix( mix(a, b, f.x), mix(c, d, f.x), f.y ) * 0.5;

}


float brickMortarLines(vec2 p, float width) {
	
  	vec2 q = floor(p);
  	float xo = odd(q.y);	
  	q.x = floor((q.x / width - (xo * 0.5)));

	p = p / vec2(width, 1.0) - vec2(xo * 0.5, 0.0);

	float t = 0.1;
	float x = 1.0 - smoothstep(0.0, t/width, min(1.0, (abs(p.x - q.x))));
	float y = 1.0 - smoothstep(0.0, t, min(1.0, (abs(p.y - q.y))));

	return max(x, y);

}


vec2 brickLuminance(vec3 p) {

  vec2 q = vec2( (atan(p.z/p.x) * 0.5945) * 15.0, p.y );

  float w = 2.0;
  float n0 = 0.25 + brickValueNoise2D(q, w) * 0.65;
  float n1 = brickMortarLines(q, w);

  return vec2(n0, n1);

}


// SCENE OBJECTS



vec3 getLightBeam(float i)
{
    
    float t = iTime * 6.283184 / 20.0;
    vec2 q = vec2(cos(t), sin(t));
        
    if (i != 0.0) {
    
       mat2 r = rotationTriple(63.0 * abs(i), 16.0 * i, 65.0 * abs(i));
       q = q * r;
    
    }
    
    return vec3(q.x, 0.0, q.y);
    
}


vec3 sdLighhouseBeam( vec3 p )
{
 
    p = p - lighthouseLantern;
    
    float t = iTime * 6.283184 / 20.0;
    vec3 q = vec3(cos(t), 0.0, sin(t));
    
    float toLantern = length(p);
    float r = 1.0 + toLantern * (0.03);

    float n = GradientNoise2D( vec2(toLantern, p.z) * 0.02, 4, 0.5 );
    r += n * (0.05 + toLantern * 0.01);

    vec3 beam = q;
    float d1 = sdCapsule( p, -beam * 0.1, beam * 3000.0, r + sin(toLantern * 0.36 + iTime * 0.1) * 0.05 );
    
    beam.xz = q.xz * rotationTriple(60.0, -11.0, 61.0);
    float d2 = sdCapsule( p, -beam * 0.1, beam * 3000.0, r + sin(toLantern * 0.15 + 0.3456 + iTime * 0.2) * 0.06 );
    
    beam.xz = q.xz * rotationTriple(60.0, 11.0, 61.0);
    float d3 = sdCapsule( p, -beam * 0.1, beam * 3000.0, r + sin(toLantern * 0.21 - 0.7456 + iTime * 0.168) * 0.04 );

    // float d = min(d1, min(d2, d3));

    return vec3(d1, d2, d3);

}


float sdLighthousePole( vec3 p )
{

    float a = sdCylinder(p - vec3(0, -0.2, 0), vec2(0.1 - p.y * 0.05, 0.05)) - 0.01;
    float b = sdCylinder(p, vec2(0.15, 0.7)) - 0.01;
    
    
    return smin(a, b, 1.0);

}


vec2 sdLighthouseBase( vec3 p )
{

    vec3 q = p - vec3(1.0, -13.5, 3.0);

    float height = 6.6;
    vec3 h = vec3(0, 0, 3);
    float base = sdCylinder(q - clamp(q, -h, h), vec2(9.0, height)) - 0.1;

    base = min(base, sdLighthousePole(p - vec3(4.0,  -height, 12.5)));
    base = min(base, sdLighthousePole(p - vec3(2.0,  -height, 13.2)));
    base = min(base, sdLighthousePole(p - vec3(0.0,  -height, 13.5)));
    base = min(base, sdLighthousePole(p - vec3(-2.0, -height, 13.2)));
    base = min(base, sdLighthousePole(p - vec3(-3.8, -height, 12.1)));
    base = min(base, sdLighthousePole(p - vec3(-5.5, -height, 10.5)));
    base = min(base, sdLighthousePole(p - vec3(-6.4, -height, 8.0)));
    base = min(base, sdLighthousePole(p - vec3(-6.4, -height, 5.5)));
    base = min(base, sdLighthousePole(p - vec3(-6.4, -height, 3.0)));
    base = min(base, sdLighthousePole(p - vec3(-6.4, -height, 0.4)));
    base = min(base, sdLighthousePole(p - vec3(-6.3, -height, -2.2)));
    base = min(base, sdLighthousePole(p - vec3(-5.5, -height, -4.7)));
    base = min(base, sdLighthousePole(p - vec3(-4.0, -height, -6.6)));
    base = min(base, sdLighthousePole(p - vec3(-2.1, -height, -7.9)));
    base = min(base, sdLighthousePole(p - vec3(0,    -height, -8.5)));
    base = min(base, sdLighthousePole(p - vec3(2.0,  -height, -8.5)));
    base = min(base, sdLighthousePole(p - vec3(4.0,  -height, -7.9)));
    base = min(base, sdLighthousePole(p - vec3(6.0,  -height, -6.6)));
    base = min(base, sdLighthousePole(p - vec3(7.6,  -height, -4.6)));
    base = min(base, sdLighthousePole(p - vec3(8.3,  -height, -2.5)));
    base = min(base, sdLighthousePole(p - vec3(8.5,  -height, -0.0)));
    base = min(base, sdLighthousePole(p - vec3(8.5,  -height, 2.5)));
    
    return vec2(base, MATERIAL_LIGHTHOUSE);

}


vec2 sdLighthouseTower( vec3 p )
{

    float tower = sdCylinder(p, vec2(4.0, 20.2));
    tower += p.y * 0.02 + exp((p.y + 30.0) * 0.06) * 0.02;
    
    float towerbase = sdCylinder(p - vec3(0, -6.8, 0), vec2(4.2, 0.25)) - 0.1;
    tower = smin(tower, towerbase, 0.1);
    
    // Silo
    float silo = sdCylinder(p - vec3(2.5, -3.9, -1.0), vec2(2.2, 3.4)) - 0.1;
    tower = min(tower, silo);
    
    
    // Platform
    
    float platform = sdCylinder(p - vec3(0, 20.0, 0), vec2(4.2, 0.25)) - 0.1;
    float underPlatform = sdCylinder(p - vec3(0, 19.5, 0), vec2(3.4, 0.1)) - 0.1;
    
    tower = smin(platform, smin(tower, underPlatform, 0.25), 0.3);
    
    // float a = pow(abs(sin(9.0 * p.y)), 20.0);         
    // float of = 1.57 * ceil(mod((18.0/17.0) * (6.0/5.0) * 2.25 * p.y - 1.57, 2.0))-1.0;
    // float an = atan(p.z/p.x);
    // float b = pow(abs(sin(10.0 * an + of)), 100.0);
    
    // tower += (a + (1.0 - a) * b) * 0.01;
        
    // Lantern Base
    
    float lanternBase = sdCylinder(p - vec3(0, 21.6, 0), vec2(2.8, 1.5));
    tower = min(tower, lanternBase);
    
    float lanternBaseRim = sdCylinder(p - vec3(0, 23.1, 0), vec2(3.05, 0.1)) - 0.04;  
    tower = min(tower, lanternBaseRim);
    
    vec2 Tower = vec2(tower, MATERIAL_LIGHTHOUSE);
    
    // Windows
    
    float offset = -1.8;
    float step = 3.8;
    
    for (int i=0; i<6; i++) {
    
        float windowFrame = sdBox(vec3(-p.x, offset-p.y+step*float(i), -4.4-p.z+(p.y+20.0)*0.03), vec3(0.6, 0.85, 0.21));
        Tower = pickClosest( vec2(windowFrame, MATERIAL_LIGHTHOUSE_WINDOW), Tower );

        float window = sdBox(vec3(-p.x, offset-p.y+step*float(i), -4.0-p.z+(p.y+5.0)*0.03), vec3(0.45, 0.7, 0.20)) + 0.05;
        // tower = max(-window, tower);
        Tower = pickFarthest( vec2(-window, MATERIAL_LIGHTHOUSE_WINDOW), Tower );
    
    }
    
    return Tower;

}


vec2 sdLighthouse( vec3 p )
{

    // Foundation
    vec2 lighthouse = sdLighthouseBase(p);
    
    
    // Body
    vec2 tower = sdLighthouseTower(p);
    lighthouse = pickClosest(lighthouse, tower);
    
    float n = GradientNoise2D( vec2(atan(p.z/p.x) / 3.141592 * 7.0, p.y * 0.5), 5, 0.5 );
    lighthouse.x += n * 0.1;
    
    for (int i=0; i<15; i++) {
    
        float an = float(i) * 3.141592 / 14.0;
        vec2 pc = vec2(cos(an), sin(an));
        
        vec2 fencepost = vec2( sdVerticalCapsule(p - vec3(4.0 * pc.x, 20.4, 4.0 * pc.y), 1.4, 0.05), MATERIAL_LIGHTHOUSE );
        lighthouse = pickClosest(lighthouse, fencepost);
        
        fencepost = vec2( sdVerticalCapsule(p - vec3(4.0 * -pc.x, 20.4, 4.0 * -pc.y), 1.4, 0.05), MATERIAL_LIGHTHOUSE );
        lighthouse = pickClosest(lighthouse, fencepost);
        
    }
    
    vec2 q1 = vec2( length(p.xz) - 4.0, p.y - 21.8 );
    
    vec2 fence = vec2( sdCircle(q1, 0.05), MATERIAL_LIGHTHOUSE );
    lighthouse = pickClosest(lighthouse, fence);
    
    
    // Lantern
    
    vec2 lantern = vec2( sdCylinder(p - vec3(0, 24.9, 0), vec2(2.1, 1.7)), MATERIAL_LIGHTHOUSE );
    //lantern.x += (p.y - 20.0) * 0.05;
    lantern.x -= pow(abs(sin(6.0 * atan(p.z / p.x))), 20.0) * 0.1;
    
    lighthouse = pickClosest(lighthouse, lantern);
    
    vec2 lanternRoofRim = vec2( sdCylinder(p - vec3(0, 26.9, 0), vec2(2.4, 0.05)) - 0.05, MATERIAL_LIGHTHOUSE );
    lighthouse = pickClosest(lighthouse, lanternRoofRim);
    
    float lanternHalfDome = sdSphere(p - vec3(0, 26.9, 0), 2.05); 
    lanternHalfDome -= pow(abs(sin(6.0 * atan(p.z / p.x))), 20.0) * 0.07;
    
    float lanternTop = sdSphere(p - vec3(0, 29.7, 0), 0.5);
    vec2 lanternRoof = vec2(smin(lanternHalfDome, lanternTop, 0.75), MATERIAL_LIGHTHOUSE );
    
    lighthouse = pickClosest(lighthouse, lanternRoof);
    
    return lighthouse;

}


vec2 sdSeaSurface( vec3 p )
{
    
    // return vec2(p.y, MATERIAL_SEA);
    return vec2(p.y - waterMotion(p * vec3(29.0, 1.0, 29.0), iTime) * 0.05, MATERIAL_SEA);

}


// RENDERING FUNCTIONS


vec2 scenePoint( vec3 p )
{
 
    vec2 d = vec2(MAX_DIST, MATERIAL_SKY);
 
    vec3 q0 = (p - lighthouseOrigin);
    vec2 q = mat2(0.7071, -0.7071, 0.7071, 0.7071) * q0.xz;
    
    float lighthouseBounds = sdBox(vec3(q.x, q0.y, q.y), vec3(40.0, 40.0, 40.0));
    
    if (lighthouseBounds < 0.0) {
    
        d = pickClosest(d, sdLighthouse(vec3(q.x, q0.y, q.y)));
    
    }

    return pickClosest( sdSeaSurface(p), d);

}


vec2 castRay( vec3 ro, vec3 rd, out float t )
{

    vec2 d = scenePoint(ro);
    t += d.x;
    
    vec3 rp = vec3(0.0);
   
    for ( int i=0; i<70; i++)
    {
    
        rp = ro + rd * t;
        d = scenePoint(rp);
        
        t += d.x;
        
        if (t >lighthouseOrigin.z + 30.0 && rd.y > 0.0) {
        
            d = vec2(MAX_DIST, MATERIAL_SKY);
            t = MAX_DIST;
            break;
        
        }
        
        if (abs(d.x) < 0.00001 || t > MAX_DIST) {
        
            break;
        
        }
    
    }
    
    return d;

}


float calcShadow( vec3 rp, vec3 rl, vec3 n )
{

    vec3 nro = rp + n * 0.01;
    
    float t = 0.0;
    vec2 d = castRay(nro, rl, t);
    
    if (t < MAX_DIST) {
    
        return 1.0 / (1.0 + (pow(t, 2.0) * 0.001));
    
    }
    
    return 0.0;

}


// https://iquilezles.org/articles/normalsSDF
vec3 calcNormal( in vec3 pos, float t )
{
    vec2 e = vec2(1.0,-1.0)*0.00001 * t;
    return normalize( e.xyy * scenePoint( pos + e.xyy ).x + 
					  e.yyx * scenePoint( pos + e.yyx ).x + 
					  e.yxy * scenePoint( pos + e.yxy ).x + 
					  e.xxx * scenePoint( pos + e.xxx ).x );   
}


float pickClosestLightBean(vec3 b)
{
    return min(b.x, min(b.y, b.z));
}


float getLighting(vec3 ro, vec3 rd, vec3 rp, vec3 n)
{

    vec3 light = normalize( vec3(-1.5, -2.0, -3.5) ) * 1.5;
    // vec3 light = normalize( vec3(-0.5, -2.0, 0.5) ) * 1.5;

    float dif = max( 0.0, -dot( light, n ) );
    
    float shd = 1.0 - calcShadow(rp, -light, n);
        
    float occ = 0.5 + n.y * 0.5;
    
    // lighthouse beams
    
    float beamLight = 0.0;
        
    vec3 toLantern = rp - lighthouseLantern;
    float toLanternDist = length(toLantern);
    toLantern = normalize(toLantern);
    
    if (rp.y > 8.0 && rp.y < 45.0) {
    
        dif += dot(toLantern, n) * 1.5;
    
    }
    
    float lighthouseOcclusion = smoothstep(10.0, 120.0, toLanternDist);
    float directIllumination = max(0.0, -dot(toLantern, n) );
        
    float lightBeamDistance = pickClosestLightBean(sdLighhouseBeam(rp));
    
    beamLight += exp(-lightBeamDistance * 0.01) * directIllumination * lighthouseOcclusion * 0.3333;
    
    if (lightBeamDistance < 0.0 && toLanternDist < 5.0)
    {
        
        float lanternGlow = smoothstep(0.1, 1.0, -lightBeamDistance);
        beamLight += lanternGlow * 1.0;
    
    }
    
    return (dif * 0.75) * (0.3 + shd * 0.7) + occ * (0.2 + shd * 0.3) + beamLight * 2.5;

}


vec2 getLighthouseLight(vec3 ro, vec3 rd)
{
    
    vec3 b = sdLighhouseBeam(ro);
    float d = pickClosestLightBean(b);
    
    float t = d;
    vec3 rp = ro + rd * t;
    
    for (int i=0; i<100; i++) {
        
        b = sdLighhouseBeam(rp);
        d = pickClosestLightBean(b);
        
        t += d;
        
        if (d < 0.01 || t >= MAX_DIST) {
            
            break;
            
        }
        
        rp = ro + rd * t;
    
    }
    
    vec2 e = vec2(1.0,-1.0)*0.001 * t;
    
    vec3 bn0 = sdLighhouseBeam( rp + e.xyy );
    vec3 bn1 = sdLighhouseBeam( rp + e.yyx );
    vec3 bn2 = sdLighhouseBeam( rp + e.yxy );
    vec3 bn3 = sdLighhouseBeam( rp + e.xxx );
    
    vec3 n1 = normalize( e.xyy * bn0.x + 
                         e.yyx * bn1.x + 
					     e.yxy * bn2.x + 
					     e.xxx * bn3.x );   
    float a1 = pow(max(0.0, dot(rd, -n1)), 2.0);
    
    vec3 n2 = normalize( e.xyy * bn0.y + 
                         e.yyx * bn1.y + 
					     e.yxy * bn2.y + 
					     e.xxx * bn3.y );   
    float a2 = pow(max(0.0, dot(rd, -n2)), 2.0);
    
    vec3 n3 = normalize( e.xyy * bn0.z + 
                         e.yyx * bn1.z + 
					     e.yxy * bn2.z + 
					     e.xxx * bn3.z );   
    float a3 = pow(max(0.0, dot(rd, -n3)), 2.0);
    
    //float a = (a1 + a2 + a3) * 0.3333;
    a1 = 0.4 + a1 * 0.6;
    a2 = 0.4 + a2 * 0.6;
    a3 = 0.4 + a3 * 0.6;
    
    float a = a1 * a2 * a3;
    
    float n = GradientNoise2D( vec2(rp.x + iTime * 2.0, rp.z) * 0.05, 4, 0.5 );
    a *= (0.5 + n * 0.5);
    
    float beamLight = exp(-(t - 0.0) * 0.002);
    
    float light = smoothstep(0.1, 1.0, beamLight * smoothstep(0.1, 1.0, 0.25 + a * 0.9));
    
    return vec2(t, light);

}


vec4 getMaterial(vec3 ro, vec3 rd, vec3 rp, vec3 n, float t, vec2 ob) 
{
    
    float l = 0.5;
    float detail = exp(-t * 0.005);
    // vec3 light = normalize( vec3(-1.0, -1.0, 0.5) );

    // Sky color
    vec3 brickColor = vec3(0.32, 0.28, 0.25);
    vec3 skycolor = vec3(0.825, 0.8, 0.925) * 0.9 - rd.y * 0.5 + rd.x * 0.3;
    vec3 col = skycolor;
    
    vec3 beam = getLightBeam(0.0);

    // Sea color
    if (t < MAX_DIST && ob.x < 0.1 )
    {
        
        vec3 q = rp - lighthouseOrigin;
        q.xz = mat2(0.7071, -0.7071, 0.7071, 0.7071) * q.xz;
        
        if (ob.y == MATERIAL_SEA)
        {
            
            float ht = 1.0 - exp(-rp.z * 0.005);
            col = mix(vec3(0.05, 0.25, 0.3), vec3(0.1, 0.15, 0.3), ht);

            float d = length(rp.xz - lighthouseOrigin.xz);
            d = 1.0 - smoothstep(0.0, 35.0, d);
            
            float ns = GradientNoise2D( vec2(rp.x + iTime * 2.5, rp.z) / 1.0, 5, 0.9 );
            // ns = smoothstep(0.0, 1.0, ns);
            
            col = mix(col, vec3(1.0 + pow(d, 2.0) * 2.5 * ns), ns * pow(d, 0.5));
        
        } 
        else if (ob.y == MATERIAL_LIGHTHOUSE) 
        {
                
            float n0 = GradientNoise2D( vec2(atan(q.z/q.x) / 3.141592 * 7.0, q.y * 0.05), 4, 0.5 );
            float n1 = GradientNoise2D( vec2(atan(q.z/q.x) / 3.141592 * 9.0, q.y * 0.5), 4, 0.5 );
                            

            if (rp.y < 9.1) 
            {
                // foundation - dark
                
                vec2 brick = brickLuminance(q * vec3(1.0, 1.5, 1.0) - vec3(0.0, -1.4, 0.0));
                col = brickColor * brick.x;
                col += brick.y * 0.75 * detail;
                
                vec3 rust = mix(vec3(0.15, 0.1 + n0 * 0.1, 0.1), vec3(0.2 + n1 * 0.15, 0.1, 0.05), smoothstep(0.25, 0.9, n1));
                col = mix(col, rust, smoothstep(0.5, 0.75, n0) * 0.75);
            
            }
            else if (rp.y < 21.41 && rp.y > 9.1 && length(q.xz) <= 5.5) 
            {
                // tower - dark section
                
                vec2 brick = brickLuminance(q * vec3(2.0, 3.0, 2.0) - vec3(0.0, -1.65, 0.0));
                col = brickColor * brick.x;
                col += brick.y * 0.95 * detail;
                
                col = col + smoothstep(0.4, 0.7, n0) * 0.2;
                col = mix(col, vec3(0.4, 0.2, 0.05), smoothstep(0.4, 0.9, n1) * 0.75);
            
            }
            else if (rp.y < 35.6 && rp.y > 21.41)
            {

                // tower - white section
                
                col = vec3(0.95);
                vec2 brick = brickLuminance(q * vec3(2.0, 3.0, 2.0) - vec3(0.0, -1.65, 0.0));
                
                col = col * 0.85 + brick.x * 0.15;
                col -= brick.y * 0.3 * detail;
                
                col = col - smoothstep(0.5, 0.7, n0) * 0.1;
                
                // stripes
                if ((rp.y > 24.5 && rp.y < 24.8) || (rp.y > 27.5 && rp.y < 27.8)) 
                {
                    
                    col = col * 0.1;
                
                }
                
                // letters
                
                vec2 lpos = vec2( atan(q.z/q.x) * 1.9 + 0.45, (rp.y - 23.95) / 2.2 );
                
                if (paintedLetters(lpos) < 0.0) {
    
                    col = col * 0.1;
    
                }

                
            }
            else if (rp.y < 39.25 && rp.y > 36.4 && length(q.xz) <= 4.0) 
            {
                // tower - lanter base
                
                vec2 brick = brickLuminance(q * vec3(1.0, 2.0, 1.0) - vec3(0.0, -0.25, 0.0));
                col = brickColor * brick.x;
                col += brick.y * 0.65 * detail;
            
            }
            
            else if (rp.y < 42.8 && rp.y > 39.25)
            {
                
                // lantern
                
                float ribs = pow(abs(sin(6.0 * atan(q.z / q.x))), 20.0);
                
                vec3 lanternCol = mix(vec3(0.95, 0.7, 0.65), vec3(0.95, 0.9, 0.85), smoothstep(0.0, 2.0, abs(rp.y - lighthouseLantern.y)) );
                
                col = mix(lanternCol, skycolor, 0.25);
                
                col = mix(lanternCol * 2.0, vec3(0.1), smoothstep(0.5, 0.75, ribs));
                
                float an = atan(q.z/q.x);
                
                float a = dot(beam, -vec3(cos(an), 0, sin(an)));
                
                col = col * (1.0 + a * 0.25);
            
            }

            else
            {
                
                float ns = GradientNoise2D( vec2(q.x, q.z) / 15.0, 5, 0.65 );
                vec3 rust = mix(vec3(0.15, 0.1 + ns * 0.075, 0.05), vec3(0.3 - ns * 0.1, 0.14, 0.05), smoothstep(0.15, 0.65, n1));
                
                col = vec3(0.025, 0.09, 0.05) - ns * 0.25;
                col = mix(col, rust, smoothstep(0.5, 0.9, ns));
                
            
            }
        
        }
        else if (ob.y == MATERIAL_LIGHTHOUSE_WINDOW) 
        {
            
            if (length(q.xz) + q.y * 0.03 < 3.77) {
            
            // if (abs(sin(rp.y * 7.68 - 0.75)) > 0.9) {
                if (length(q.xz) <= 5.0 && (abs(sin(atan(q.z/q.x) * 48.0 + 0.1)) > 0.85 || abs(sin(rp.y * 7.65 - 0.5)) > 0.95)) {

                    col = vec3(0.75);

                }
                else if (abs(q.y - 4.0) < 4.0)
                {
                    col = vec3(0.95, 0.9, 0.85) * 3.0;
                }
                else
                {
                
                    col = vec3(0.05, 0.02, 0.1);
                
                }
                
                
            }
            else
            {
            
                if (rp.y < 19.0) {
            
                    col = brickColor;
                    
                }
                else
                {
                
                    col = vec3(0.75);
                
                }
            }
        
        }
        else if (ob.y == MATERIAL_LIGHTHOUSE_BEAM)
        {
            
            col = vec3(1.0, 0.98, 0.95) * 1.0;
            
        }
        
                
        float occ = 0.5 + n.y * 0.5;
        col = mix(col, skycolor, occ * 0.3333);
        
        l = getLighting(ro, rd, rp, n);
        
    }
    
    float fog = smoothstep(140.0, 300.0, t) - abs(rd.y) * 1.0;
    
    float ns = GradientNoise2D( vec2(rp.x + iTime * 50.0, rp.y) * 0.0015, 4, 0.5 );
        
    vec3 fogColor = vec3(0.95, 0.97, 1.0) * skycolor + 0.6 * (0.6 * ns - 0.2);
    
    col = mix(col, smoothstep(0.1, 1.0, fogColor), fog);
    
    l = mix(l, 1.0, fog);
    
    vec2 lhl = getLighthouseLight(ro, rd);
    
    if (lhl.x < t) {
        
        col = col * (1.0 + lhl.y);
    
    }
    
    
    vec3 toLantern = normalize(lighthouseLantern - ro);
    float beamblast = max(0.0, dot(beam, -toLantern)) * max(0.0, dot(toLantern, vec3(0.0, 0.0, 1.0)));
    beamblast = smoothstep(0.75, 1.0, beamblast);
    
    float depth = exp(-t * 0.003);
    col = col * (1.0 + (beamblast * 0.1 * depth));
    
    // col = smoothstep(0.15, 1.0, col);
    
    return vec4(col, l);

}



vec3 render( vec3 ro, vec3 rd, float t, vec2 ob ) {
            
    vec3 rp = ro + rd * t;
    
    vec3 n = calcNormal(rp, t);
    vec4 mt = getMaterial(ro, rd, rp, n, t, ob);
    
    vec3 col = mt.rgb * mt.a;
    
    float fogT = smoothstep(0.0, 500.0, t);
    
    if (ob.y == MATERIAL_SEA && fogT < 1.0)
    {
            
        vec3 rrd = reflect(rd, n);
        vec3 rro = rp + n * 0.01;
            
        float rt = 0.0;
        vec2 rob = castRay(rro, rrd, rt);
        
        vec3 rrp = rro + rrd * rt;
        
        vec3 rn = calcNormal(rrp, rt);
        vec4 rmt = getMaterial(rro, rrd, rrp, rn, rt, rob);
        vec3 rcol = rmt.rgb * rmt.a;
        
        col = mix(col - 0.1 * (1.0 - fogT), rcol, 0.3 * (1.0 - fogT));
        
    }

    return col;

}



void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    
    fragCoord = fragCoord * 1.2;

    // Normalized pixel coordinates (from -1 to 1)
    vec2 uv = ((2.0 * fragCoord) - iResolution.xy) / iResolution.y;
    
    // Camera
    
    // 15.0, 0.2
    float cameraHeight = 15.0;
    float cameraPitch = 0.2;
    
    vec3 ro = vec3(0, cameraHeight, -4.0);
    vec3 sp = vec3(0, cameraHeight + cameraPitch, 0);
    vec3 rd = normalize( vec3(uv, 0.0) + sp - ro);

    // Ray marching to the scene
    float t = 0.0;
    vec2 ob = castRay(ro, rd, t);

    // Rendering the scene
    vec3 col = render(ro, rd, t, ob);
    
    
    float L = dot(smoothstep(0.05, 1.0, col), vec3(0.299, 0.587, 0.114));
    
    col = col * vec3(1.0, 0.98, 0.9);
    col = mix(col, vec3(L), 0.5);
    
    col = mix(col, smoothstep(0.0, 0.95, col), 0.25);
    
    /*if (iTime < 8.0 && paintedLetters(uv * vec2(1.4, 1.2) - vec2(-0.08, -0.75)) < 0.0) {
    
        col = mix(vec3(1.0), col, smoothstep(7.0, 7.5, iTime));
    
    }*/

    // Output to screen
    fragColor = vec4(col,1.0);
}