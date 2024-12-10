#define PI 3.14159265

#define saturate(x) clamp(x,0.,1.)
#define SUNDIR normalize(vec3(0.2,.3,2.))
#define FOGCOLOR vec3(1.,.2,.1)

float smin( float a, float b, float k );
float smax( float a, float b, float k );
float box( vec3 p, vec3 b, float r );
float capsule( vec3 p, float h, float r );
vec3 hash( uint n );
mat2 rot(float v);


float time;

float train(vec3 p) {
    
    // base 
    float d = abs(box(p-vec3(0., 0., 0.), vec3(100.,1.5,5.), 0.))-.1;
    
    // windows
    d = smax(d, -box(p-vec3(1.,0.25,5.), vec3(2.,.5,0.0), .3), 0.03);
    d = smax(d, -box(p-vec3(-3.,0.25,5.), vec3(.2,.5,0.0), .3), 0.03);
    d = smin(d,  box(p-vec3(1.,0.57,5.), vec3(5.3,.05,0.1), .0), 0.001);
    
    // seats
    p.x = mod(p.x-.8,2.)-1.;
    p.z = abs(p.z-4.3)-.3;
    d = smin(d, box(p-vec3(0.,-1., 0.), vec3(.3,.1-cos(p.z*PI*4.)*.01,.2),.05), 0.05);
    d = smin(d, box(p-vec3(0.4+pow(p.y+1.,2.)*.1,-0.38, 0.), vec3(.1-cos(p.z*PI*4.)*.01,.7,.2),.05), 0.1);
    d = smin(d, box(p-vec3(0.1,-1.3, 0.), vec3(.1,.2,.1),.05), 0.01);

    return d;
}

float catenary(vec3 p) {
    
    p.z -= 12.;
    vec3 pp = p;
    p.x = mod(p.x,100.)-50.;
    
    // base
    float d = box(p-vec3(0.,0.,0.), vec3(.0,3.,.0), .1);
    d = smin(d, box(p-vec3(0.,2.,0.), vec3(.0,0.,1.), .1), 0.05);
    p.z = abs(p.z-0.)-2.;
    d = smin(d, box(p-vec3(0.,2.2,-1.), vec3(.0,0.2,0.), .1), 0.01);
    
    // lines
    pp.z = abs(pp.z-0.)-2.;
    d = min(d, capsule(p-vec3(-50.,2.4-abs(cos(pp.x*.01*PI)),-1.),10000.,.02));
    d = min(d, capsule(p-vec3(-50.,2.9-abs(cos(pp.x*.01*PI)),-2.),10000.,.02));
    
    return d;
}


float city(vec3 p) {
    vec3 pp = p;
    ivec2 pId = ivec2((p.xz)/30.);
    vec3 rnd = hash(uint(pId.x + pId.y*1000));
    p.xz = mod(p.xz, vec2(30.))-15.;
    float h = 5.+float(pId.y-3)*5.+rnd.x*20.;
    float offset = (rnd.z*2.-1.)*10.;
    float d = box(p-vec3(offset,-5.,0.), vec3(5.,h,5.), 0.1);
    d = min(d, box(p-vec3(offset,-5.,0.), vec3(1.,h+pow(rnd.y,4.)*10.,1.), 0.1));
    d = max(d,-pp.z+100.);
    d = max(d,pp.z-300.);
    
    return d*.6;
}

float map(vec3 p) {
    float d = train(p);
    p.x -= mix(0.,time*15., saturate(time*.02));
    d = min(d, catenary(p));
    d = min(d, city(p));
    d = min(d, city(p+15.));
    return d;
}

float shadow( in vec3 ro, in vec3 rd, in float mint, in float tmax )
{
    float t = mint;
    for(int i=0; i<128; i++) {
        float d = map(ro+rd*t);
        if (abs(d)<0.01) break;
        t += d;
        if (t > 300.) return 1.;
    }
    return 0.;
}


vec3 randomSphereDir( vec2 rnd )
{
    float s = rnd.x*PI*2.;
    float t = rnd.y*2.-1.;
    return vec3(sin(s), cos(s), t) / sqrt(1.0 + t * t);
}

float hash( float p ) 
{
    return fract(sin(p)*43758.5453123);
}

vec3 randomHemisphereDir( vec3 dir, float i )
{
    vec3 v = randomSphereDir( vec2(hash(i+1.), hash(i+2.)) );
    return v * sign(dot(v, dir));
}
float ambientOcclusion( vec3 p, vec3 n, float maxDist, float falloff )
{
	const int nbIte = 16;
    const float nbIteInv = 1./float(nbIte);
    const float rad = 1.-1.*nbIteInv; //Hemispherical factor (self occlusion correction)
    
	float ao = 0.0;
    
    for( int i=0; i<nbIte; i++ )
    {
        float l = hash(float(i))*maxDist;
        vec3 rd = normalize(n+randomHemisphereDir(n, l )*rad)*l; // mix direction with the normal
        													    // for self occlusion problems!
        
        ao += (l - max(map( p + rd ),0.)) / maxDist * falloff;
    }
	
    return clamp( 1.-ao*nbIteInv, 0., 1.);
}

// https://iquilezles.org/articles/smin
float smin( float a, float b, float k )
{
    float h = max(k-abs(a-b),0.0);
    return min(a, b) - h*h*0.25/k;
}

// https://iquilezles.org/articles/smin
float smax( float a, float b, float k )
{
    k *= 1.4;
    float h = max(k-abs(a-b),0.0);
    return max(a, b) + h*h*h/(6.0*k*k);
}

float box( vec3 p, vec3 b, float r )
{
  vec3 q = abs(p) - b;
  return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0) - r;
}

float capsule( vec3 p, float h, float r )
{
  p.x -= clamp( p.x, 0.0, h );
  return length( p ) - r;
}

vec3 hash( uint n ) 
{
    // integer hash copied from Hugo Elias
	n = (n << 13U) ^ n;
    n = n * (n * n * 15731U + 789221U) + 1376312589U;
    uvec3 k = n * uvec3(n,n*16807U,n*48271U);
    return vec3( k & uvec3(0x7fffffffU))/float(0x7fffffff);
}

float hash2Interleaved( vec2 x )
{
    // between random & dithered pattern
    // good for jittering and blur as well as blue noise :)
    // http://www.iryoku.com/next-generation-post-processing-in-call-of-duty-advanced-warfare
    vec3 magic = vec3( 0.06711056, 0.00583715, 52.9829189 );
    return fract( magic.z * fract( dot( x, magic.xy ) ) );
}

mat2 rot(float v) {
    float a = cos(v);
    float b = sin(v);
    return mat2(a,-b,b,a);
}