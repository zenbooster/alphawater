//Render particles
vec2 R;
vec4 T ( vec2 U ) {return texture(iChannel0,U/R);}
vec4 P ( vec2 U ) {return texture(iChannel1,U/R);}
vec4 D ( vec2 U ) {return texture(iChannel2,U/R);}
void mainImage( out vec4 C, in vec2 U )
{	R = iResolution.xy;
    C = P(U);
 	C = vec4(vec3(smoothstep(1.5,0.5,length(C.xy-U))),1);
 	C = C+vec4(0.995,0.98,0.95,1.)*(D(U));
 	if(iFrame < 1) C = vec4(0);
}
