// Fork of "Super SPH " by michael0884. https://shadertoy.com/view/tdXBRf
// 2020-05-09 15:47:15


vec3 ACESFilm(vec3 x){
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return (x*(a*x+b))/(x*(c*x+d)+e);
}

mat3 gaussianFilter = mat3(41, 26, 7,
                           26, 16, 4,
                           7,  4,  1) / 273.;

vec3 bloom(float scale, vec2 fragCoord){
    float logScale = log2(scale);
    vec3 bloom = vec3(0);
    for(int y = -2; y <= 2; y++)
        for(int x = -2; x <= 2; x++)
            bloom += gaussianFilter[abs(x)][abs(y)] * textureLod(iChannel0, (fragCoord+vec2(x, y)*scale)/iResolution.xy, logScale+2.).rgb;
    
    return bloom;
}


void mainImage( out vec4 fragColor, in vec2 pos )
{
    sN = SN; 
    N = ivec2(R*P/vec2(sN));
    TN = N.x*N.y;
    ivec2 pi = ivec2(floor(pos));
    
     vec3 bloomSum = vec3(0.);
    bloomSum += bloom(.4 * R.y, pos) * .07;
    bloomSum += bloom(.2 * R.y, pos) * .07;
    
    fragColor.xyz = texel(ch2, pi).xyz + 1.*bloomSum;
    fragColor.xyz = ACESFilm(fragColor.xyz);
}