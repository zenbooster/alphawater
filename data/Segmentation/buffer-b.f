#define R iResolution.xy
#define A(U) texture(iChannel0,(U)/R)
#define B(U) texture(iChannel1,(U)/R)
#define C(U) texture(iChannel2,(U)/R)
#define D(U) texture(iChannel3,(U)/R)
void mainImage( out vec4 Q, vec2 U )
{
    Q = A(U);
    vec4
        n = A(U+vec2(0,1)),
        e = A(U+vec2(1,0)),
        s = A(U-vec2(0,1)),
        w = A(U-vec2(1,0));
    Q.xy = vec2(-e.x+w.x,-n.x+s.x);
    if (length(Q.xy)>0.) Q.xy = mix(Q.xy,normalize(Q.xy),.03);
}