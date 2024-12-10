float phaseFunction(float lightDotView)
{
    const float k = .9;
	float v = 1.0 - k * k;
	v /= (4.0 * PI * pow(1.0 + k * k - (2.0 * k)*lightDotView, 1.5));
	return v;
}


void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    // early exit because we want half resolution
    vec2 invRes = vec2(1.) / iResolution.xy;
    vec2 uv = fragCoord * invRes*2.;
    if(uv.x>1. || uv.y>1. )
    {
        fragColor = vec4(0.);
        return;
    } 
    
    time = iTime;
	float l = texture(iChannel0, uv).a;
    
    vec2 v = -1.0+2.0*(uv);
	v.x *= iResolution.x/iResolution.y;
    
    vec3 ro = vec3(-1.5,-.4,1.2);
    vec3 rd = normalize(vec3(v, 2.5));
    rd.xz = rot(.15)*rd.xz;
    rd.yz = rot(.1)*rd.yz;
    
    
    float jitt = hash2Interleaved(gl_FragCoord.xy)*.2;
    const float eps = 0.2;
    
    // acc shadow loop
    float phase = phaseFunction(dot(SUNDIR,rd));
    vec3 godray = vec3(0.);
    for(float i=0.0; i<1.; i+=eps) {
       vec3 p = ro+rd*l*(i+jitt);
       float d = shadow(p, SUNDIR, float(.1), float(500.));
       godray += d * phase;
    }
	godray = godray;
        
    
    fragColor = vec4(godray,l);
}