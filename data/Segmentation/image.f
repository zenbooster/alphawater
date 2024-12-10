#define R iResolution.xy
#define A(U) texture(iChannel0,(U)/R)
#define B(U) texture(iChannel1,(U)/R)
#define C(U) texture(iChannel2,(U)/R)
#define D(U) texture(iChannel3,(U)/R)
void mainImage( out vec4 Q, vec2 U )
{
    vec4
        n = A(U+vec2(0,1)),
        e = A(U+vec2(1,0)),
        s = A(U-vec2(0,1)),
        w = A(U-vec2(1,0));
    vec3 no = normalize(vec3(-e.x+w.x,-n.x+s.x,.00001));
    vec3 re = reflect(normalize(vec3(0,0,1)),no);
    
    Q = vec4(0.7+0.5*no.x)*(0.5+.5*sin(30.*D(U).x*vec4(1,2,3,4)));
    
}