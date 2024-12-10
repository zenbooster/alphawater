// Voronoi based particle tracking

vec2 R;float N;
vec4 T ( vec2 U ) {return texture(iChannel0,U/R);}
vec4 P ( vec2 U ) {return texture(iChannel1,U/R);}
void swap (vec2 U, inout vec4 Q, vec2 u) {
    vec4 p = P(U+u);
    float dl = length(U-Q.xy) - length(U-p.xy);
    float e = .1;
    // allows for probabistic reproduction
    Q = mix(Q,p,0.5+0.5*sign(floor(1e3*dl+0.5)));
}
void mainImage( out vec4 Q, in vec2 U )
{   R = iResolution.xy;
 	U = U-2.*T(U).xy;
 	U = U-2.*T(U).xy;
 	Q = P(U);
 	swap(U,Q,vec2(1,0));
 	swap(U,Q,vec2(0,1));
 	swap(U,Q,vec2(0,-1));
 	swap(U,Q,vec2(-1,0));
 	Q.xy = Q.xy + 2.*T(Q.xy).xy;
 	Q.xy = Q.xy + 2.*T(Q.xy).xy;
 	if (Q.z == 0.) Q = vec4(floor(U/10.)*10.,U);
}