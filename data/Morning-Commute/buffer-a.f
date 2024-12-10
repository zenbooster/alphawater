


float trace(vec3 ro, vec3 rd, vec2 nearFar) {
    float t = nearFar.x;
    for(int i=0; i<128; i++) {
        float d = map(ro+rd*t);
        t += d;
        if( abs(d) < 0.001 || t > nearFar.y )
            break;
    }
    
    return t;
}
float traceFast(vec3 ro, vec3 rd, vec2 nearFar) {
    float t = nearFar.x;
    for(int i=0; i<64; i++) {
        float d = map(ro+rd*t);
        t += d;
        if( abs(d) < 0.001 || t > nearFar.y )
            break;
    }
    
    return t;
}

vec3 normal(vec3 p, float t) {
    vec2 eps = vec2(0.01, 0.);
    float d = map(p);
    
    vec3 n;
    n.x = d - map(p-eps.xyy);
    n.y = d - map(p-eps.yxy);
    n.z = d - map(p-eps.yyx);
    n = normalize(n);
    
    return n;
}

vec3 skyColor(vec3 rd) {
    vec3 col = FOGCOLOR;
    col += vec3(1.,.3,.1)*1. * pow(max(dot(rd,SUNDIR),0.),30.);
    col += vec3(1.,.3,.1)*10. * pow(max(dot(rd,SUNDIR),0.),10000.);
    return col;
}

vec3 shade(vec3 ro, vec3 rd, vec3 p, vec3 n) {
    vec3 col = vec3(0.);
    
    vec3 diff = vec3(1.,.5,.3) * max(dot(n,SUNDIR),0.);
    vec3 amb = vec3(0.1,.15,.2) * ambientOcclusion(p,n, .75,1.5);
    //float shad = shadow(p,SUNDIR, 0.1,200.);
    
    col = diff *.03;// * shad;
    col = (diff*0.3 + amb*.3)*.02;
    
    return col;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    time = iTime;
    vec2 invRes = vec2(1.) / iResolution.xy;
    vec2 uv = fragCoord * invRes;
    
    vec2 jitt = vec2(0.);
    #if 1
    vec2 blue = vec2(0.); //texture(iChannel1, (fragCoord.xy) / vec2(1024.0)).zw;
    blue = fract(blue + float(iFrame%256) * 0.61803398875);
    jitt = (blue-.5)*1. * invRes;
	#endif
    vec2 v = -1.0+2.0*(uv+jitt);
	v.x *= iResolution.x/iResolution.y;
    
    vec3 ro = vec3(-1.5,-.4,1.2);
    vec3 rd = normalize(vec3(v, 2.5));
    rd.xz = rot(.15)*rd.xz;
    rd.yz = rot(.1)*rd.yz;
    
    float t = trace(ro,rd, vec2(0.,300.));
    vec3 p = ro + rd * t;
    vec3 n = normal(p,t);
    vec3 col = skyColor(rd);
    
    if (t < 300.) {
        col = shade(ro,rd, p,n);
    
        if (p.z<6.) {
            vec3 rrd = reflect(rd,n);
            float t2 = traceFast(p, rrd, vec2(0.1,300.));
            vec3 rp = p + rrd * t2;
            vec3 rn = normal(rp,t2);
            float fre = pow( saturate( 1.0 + dot(n,rd)), 8.0 );
            vec3 rcol = skyColor(rrd);
            if (t2 < 300.) {
                rcol = shade(p,rrd, rp, rn);
            	rcol = mix(col, FOGCOLOR, smoothstep(100.,500.,t2));
            }
            col = mix(col, rcol, fre*.1);
        }


        col = mix(col, FOGCOLOR, smoothstep(100.,500.,t));
    }
    
    if (p.z<6.) {
    	fragColor = mix(texture(iChannel0, uv), vec4(col,t), 0.2);
    } else {
        fragColor = vec4(col,t);
    }
}