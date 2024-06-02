#define PI 3.141592
#define FAR 50.0 
#define EPS 0.005

#define CA vec3(0.5, 0.5, 0.5)
#define CB vec3(0.5, 0.5, 0.5)
#define CC vec3(1.0, 1.0, 1.0)
#define CD vec3(0.0, 0.33, 0.67)

const vec3 ac = vec3(1.0, 0.6, 0.05);

mat2 rot(float x) {return mat2(cos(x), sin(x), -sin(x), cos(x));}
//IQ cosine palattes
//https://iquilezles.org/articles/palettes
vec3 palette(float t, vec3 a, vec3 b, vec3 c, vec3 d) {return a + b * cos(6.28318 * (c * t + d));}

/* Distance functions IQ & Mercury */

float sdCaps(vec3 p, vec3 a, vec3 b, float r) {
    vec3 pa = p - a, ba = b - a;
    float h = clamp(dot(pa, ba) / dot(ba, ba), 0., 1.);
    return length(pa - ba * h) - r;
}

float sdSphere(vec3 p, float r) {
    return length(p) - r;    
}

float smin(float a, float b, float k) {
	float h = clamp( 0.5 + 0.5*(b-a)/k, 0.0, 1.0 );
	return mix( b, a, h ) - k*h*(1.0-h);
}

float planeIntersection(vec3 ro, vec3 rd, vec3 n, vec3 o) {
    return dot(o - ro, n) / dot(rd, n);
}

// see mercury sdf functions
// Repeat around the origin by a fixed angle.
// For easier use, num of repetitions is use to specify the angle.
float pModPolar(inout vec2 p, float repetitions) {
    float angle = 2.0 * PI / repetitions;
    float a = atan(p.y, p.x) + angle / 2.0;
    float r = length(p);
    float c = floor(a / angle);
    a = mod(a, angle) - angle / 2.0;
    p = vec2(cos(a), sin(a)) * r;
    // For an odd number of repetitions, fix cell index of the cell in -x direction
    // (cell index would be e.g. -5 and 5 in the two halves of the cell):
    if (abs(c) >= (repetitions / 2.0)) c = abs(c);
    return c;
}

/* Model - g prefix = glow */

float dfEars(vec3 p, float T) {
    p.z = abs(p.z);
    p.yz *= rot(1.1 + sin(T * 4.0) * 0.3);
    p.xy *= rot(-0.8);
    return smin(sdCaps(p, vec3(0.0, 0.6, 0.0), vec3(0.0, 0.9, 0.0), 0.4),
                sdCaps(p, vec3(0.0, 0.0, 0.0), vec3(0.0, 0.9, 0.0), 0.1), 0.2);  
}

float gdfEars(vec3 p, float T) {
    p.z = abs(p.z);
    p.yz *= rot(1.1 + sin(T * 4.0) * 0.3);
    p.xy *= rot(-0.8);
    return sdCaps(p, vec3(0.0, 0.7, 0.0), vec3(0.0, 0.8, 0.0), 0.02);
}

float dfSnout(vec3 p) {
    p.xy *= rot(-0.2);
    return smin(sdCaps(p, vec3(-0.5, 0.0, 0.0), vec3(-0.9, 0.0, 0.0), 0.4),
                sdCaps(p, vec3(0.0, 0.0, 0.0), vec3(-1.4, 0.0, 0.0), 0.1), 0.2); 
}

float gdfSnout(vec3 p) {
    p.xy *= rot(-0.2);
    return sdCaps(p, vec3(-0.6, 0.0, 0.0), vec3(-0.8, 0.0, 0.0), 0.02);
}

float dfHead(vec3 p, float ofs, float T) {
    p.xy *= rot(0.4 + sin(T * 2.0) * 0.2);
    float neck = smin(sdCaps(p, vec3(0.0, 0.6, 0.0), vec3(0.0, 1.1, 0.0), 0.4),
                      sdCaps(p, vec3(0.0, 0.0, 0.0), vec3(0.0, 1.7, 0.0), 0.1), 0.2); 
    float head = dfSnout(p - vec3(0.0, 1.7, 0.0));
    float ears = dfEars(p - vec3(0.0, 1.7, 0.0), T);
    float knot = sdSphere(p - vec3(0.0, 1.7, 0.0), 0.16);
    return min(min(head, knot), min(neck, ears));
}

float gdfHead(vec3 p, float ofs, float T) {
    p.xy *= rot(0.4 + sin(T * 2.0) * 0.2);
    float neck = sdCaps(p, vec3(0.0, 0.8, 0.0), vec3(0.0, 0.9, 0.0), 0.02);
    float head = gdfSnout(p - vec3(0.0, 1.7, 0.0));
    float ears = gdfEars(p - vec3(0.0, 1.7, 0.0), T);
    return min(head, min(neck, ears));
}

float dfLegs(vec3 p, float a) {
    p.z = abs(p.z);
    p.yz *= rot(-0.6);
    p.xy *= rot(a);
    return smin(sdCaps(p, vec3(0.0, -0.6, 0.0), vec3(0.0, -1.6, 0.0), 0.4),
                sdCaps(p, vec3(0.0, 0.0, 0.0), vec3(0.0, -2.1, 0.0), 0.1), 0.2);
}

float gdfLegs(vec3 p, float a) {
    p.z = abs(p.z);
    p.yz *= rot(-0.6);
    p.xy *= rot(a);
    return sdCaps(p, vec3(0.0, -1.0, 0.0), vec3(0.0, -1.2, 0.0), 0.02);
}

float dfTail(vec3 p, float a) {
    p.yz *= rot(a);
    p.xy *= rot(-0.5);
    return smin(sdCaps(p, vec3(0.0, 0.6, 0.0), vec3(0.0, 1.0, 0.0), 0.4),
                sdCaps(p, vec3(0.0, 0.0, 0.0), vec3(0.0, 1.5, 0.0), 0.1), 0.2);  
}

float gdfTail(vec3 p, float a) {
    p.yz *= rot(a);
    p.xy *= rot(-0.5);
    return sdCaps(p, vec3(0.0, 0.7, 0.0), vec3(0.0, 0.9, 0.0), 0.02);
}

float dfDog(vec3 p, float ofs, float T) {
    
    p.xz *= rot(PI * -0.5);
    p.y -= 1.0 - sin((T + ofs) * 2.0); //jump
    
    float body = smin(sdCaps(p, vec3(-0.85, 0.0, 0.0), vec3(0.85, 0.0, 0.0), 0.4),
                     sdCaps(p, vec3(-1.4, 0.0, 0.0), vec3(1.4, 0.0, 0.0), 0.1), 0.2);
	body = min(body, sdSphere(p - vec3(-1.4, 0.0, 0.0), 0.16));
	body = min(body, sdSphere(p - vec3(1.4, 0.0, 0.0), 0.16));
               
    vec3 q = p;
    q.x = abs(q.x);
    float legs = dfLegs(q - vec3(1.4, 0.0, 0.0), 0.5 + sin((T + ofs) * 2.0) * 0.4);
    
    float waggle = sin(T * 8.0) * 0.4; //waggle tail
    float tail = dfTail(p - vec3(1.4, 0.0, 0.0), waggle); 
    float head = dfHead(p - vec3(-1.4, 0.0, 0.0), ofs, T);

    return min(body, min(tail, min(head, legs)));
}

float gdfDog(vec3 p, float ofs, float T) {
    
    p.xz *= rot(PI * -0.5);
    p.y -= 1.0 - sin((T + ofs) * 2.0); //jump
    
    float body = sdCaps(p, vec3(-0.5, 0.0, 0.0), vec3(0.5, 0.0, 0.0), 0.02);
               
    vec3 q = p;
    q.x = abs(q.x);
    float legs = gdfLegs(q - vec3(1.4, 0.0, 0.0), 0.5 + sin((T + ofs) * 2.0) * 0.4);
    
    float waggle = sin(T * 8.0) * 0.4; //waggle tail
    float tail = gdfTail(p - vec3(1.4, 0.0, 0.0), waggle); 
    float head = gdfHead(p - vec3(-1.4, 0.0, 0.0), ofs, T);

    return min(body, min(tail, min(head, legs)));
}

//https://www.shadertoy.com/view/lsdXDH
vec4 desaturate(vec3 c, float f) {
	vec3 lum = vec3(0.299, 0.587, 0.114);
	vec3 gray = vec3(dot(lum, c));
	return vec4(mix(c, gray, f), 1.0);
}

vec3 saturate(vec3 c) {
    return clamp(c, 0.0, 1.0);
}
