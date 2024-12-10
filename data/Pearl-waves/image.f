#define ITERS 12
const float scale = 2.;
const float scale1 = 1.15;

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    fragColor = vec4(0.0);
    vec3 col=vec3(0.0),col_prev=vec3(0.0);
    vec2 uv = -(fragCoord.yx*10.0-iResolution.yx)/iResolution.y/15.0;
    //uv.y += (iTime)/25.0;
    float s1 = (iTime/4.-uv.x)/4.;
    for(int c=0;c<ITERS;c++){
        col_prev = col;
        for(int i=0;i<ITERS;i++)
        {
            uv= -fract(-uv-((vec2(uv.x/scale-uv.y/scale1,uv.y/scale-uv.x/scale1)/(scale))))/scale1;
            uv.x *= -scale1;
            uv = fract(-uv.yx)+fract(uv/(8.)+s1);
            uv.y /= scale1;
        }
        col[2] = abs(fract(uv.y)-fract(uv.x));
        col = ((col+col_prev.yzx))/2.125;
	}
    fragColor = vec4(vec3(col*3.0),1.0);
    
}