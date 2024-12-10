void mainImage( out vec4 fragColor, in vec2 fragCoord )
{	
    vec2 invRes = vec2(1.) / iResolution.xy;
    vec2 uv = fragCoord * invRes;
    
    vec4 center = texture( iChannel0, uv);
    
    vec4 acc = vec4(center.rgb, 1.);
    if (center.w>8.) {
        const int N = 3;
        for(int j=-N; j<=N; j++)
        for(int i=-N; i<=N; i++)
        {
            vec2 offset = vec2(i,j) * invRes;
            vec4 tap = texture(iChannel0, uv + offset*.8);
            acc += vec4(tap.rgb,tap.w>8.);
        }
    }
    acc.rgb /= acc.w;
    
    fragColor = vec4(acc.rgb, center.w);

}