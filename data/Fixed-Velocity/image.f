vec2 R;
vec4 T (vec2 U) {return texture(iChannel0,U/R);}
vec4 D (vec2 U) {return texture(iChannel1,U/R);}
void mainImage( out vec4 C, in vec2 U )
{   R = iResolution.xy;
 	vec4 i = D(U),
         t = T(U);
 	vec2 d = vec2(
    	D(U+vec2(1,0)).x-D(U-vec2(1,0)).x,
    	D(U+vec2(0,1)).x-D(U-vec2(0,1)).x
    );
 	C = abs(sin(.2*sqrt(i)*vec4(1.,1.3,1.5,4)));
}