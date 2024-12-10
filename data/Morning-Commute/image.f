void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 invRes = vec2(1.) / iResolution.xy;
    vec2 uv = fragCoord/iResolution.xy;

    // Base color
    vec2 offset = (uv*2.-1.) * invRes*1.3;
    vec3 col = vec3(0.);
    col.r = texture(iChannel0, uv+offset).r;
    col.g = texture(iChannel0, uv-offset).g;
    col.b = texture(iChannel0, uv+offset).b;
    
    float t = texture(iChannel0, uv+offset).a;
    
    // blur godrays
    vec4 godray = vec4(0.);
    for(float x=-3.; x<=3.; x+=1.)
    for(float y=-3.; y<=3.; y+=1.) { 
        vec4 tap = texture(iChannel1,uv*.5+vec2(x,y)*invRes);
        float w = 1.;
        if(tap.w>t+1. && t <8.)
            w = 0.;
    	godray += vec4(tap.rgb,1.)*w;
    }
    godray /= godray.w; 
    col += FOGCOLOR*godray.rgb*.01;
    
    col = pow(col, vec3(1./2.2));
    col = pow(col, vec3(.6,1.,.8*(uv.y*.2+.8)));
    
    // Vignetting
    float vignetting = pow(uv.x*uv.y*(1.-uv.x)*(1.-uv.y), .3)*2.5;
    col *= vignetting;
    //col *= .7+texture(iChannel2, vec2(0.1,0.0)).r;
    
    // Output to screen
    fragColor = vec4(col,1.0) * smoothstep(0.,10.,iTime);
}