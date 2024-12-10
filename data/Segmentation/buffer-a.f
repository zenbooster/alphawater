#define R iResolution.xy
#define A(U) texture(iChannel0,(U)/R)
#define B(U) texture(iChannel1,(U)/R)
#define C(U) texture(iChannel2,(U)/R)
#define D(U) texture(iChannel3,(U)/R)
void mainImage( out vec4 Q, vec2 U )
{
    vec2 u = .5+.1*vec2(sin(.1*iTime),cos(.1*iTime));
    if (iMouse.z>0.) u = iMouse.xy/R;
    U -= u*R; 
    float r = length(U)/R.y,
        a = .01*sin(.3*iTime)/(1.+5.*r);

    U *= .996*mat2(cos(a),-sin(a),sin(a),cos(a));
    U += u*R;
    Q = A(U);
    vec4
        n = A(U+vec2(0,1)),
        e = A(U+vec2(1,0)),
        s = A(U-vec2(0,1)),
        w = A(U-vec2(1,0));
    Q = mix(Q,0.25*(n+e+s+w),.1);
    Q = mix(Q,D(U),.001);
    if (iFrame < 10) Q = vec4(.001)*sin(U.y/R.y*30.+U.x/R.x);
    if (iMouse.z>0.&&length(U-iMouse.xy)<2.) Q=vec4(.01);
}