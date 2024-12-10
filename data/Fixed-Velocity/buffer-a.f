vec2 R;float N;
float hash(vec2 p)
{ // Dave H
	vec3 p3  = fract(vec3(p.xyx) * .1031);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}
float ln (vec2 p, vec2 a, vec2 b) { // returns distance to line segment for mouse input
    return length(p-a-(b-a)*clamp(dot(p-a,b-a)/dot(b-a,b-a),0.,1.));
}
vec4 T ( vec2 U ) {return texture(iChannel0,U/R);}
float X (vec2 U0, vec2 U, vec2 U1, inout vec4 Q, in vec2 r) {
    vec2 V = U + r;
    vec4 t = T(V);
    vec2 V0 = V - t.xy,
         V1 = V + t.xy;
    float P = t.z, rr = length(r);
    Q.xy -= r*(P-Q.z)/rr/N;
    return (0.5*(length(V0-U0)-length(V1-U1))+P)/N;
}

void mainImage( out vec4 Q, in vec2 U )
{   R = iResolution.xy;
 	vec2 U0 = U - T(U).xy,
         U1 = U + T(U).xy;
 	float P = 0.; Q = T(U0);
 if (length(Q.xy)==0.||iFrame < 1) {
     	float h = 6.3*hash(U);
     	Q = vec4(0.4*vec2(cos(h),sin(h)),0,0);
        	
 } else {
 	N = 4.;;
    P += X (U0,U,U1,Q, vec2( 1, 0));
 	P += X (U0,U,U1,Q, vec2( 0,-1));
 	P += X (U0,U,U1,Q, vec2(-1, 0));
 	P += X (U0,U,U1,Q, vec2( 0, 1));
 	Q.z = P;
 	Q.xy=mix(Q.xy,0.4*normalize(Q.xy),0.01);
 	vec4 mo = texture(iChannel2,vec2(0));
 	float l = ln(U,mo.xy,mo.zw);
 	if (mo.z > 0. && l < 10.) Q.z -= .1*(10.-l);
 
 }
}