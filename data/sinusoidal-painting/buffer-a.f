
// move brush
vec2 move(float t)
{
    vec2 pos = vec2(0);
    
    // random targets
    float jitter = .5;
    float time = t*3.;
    float index = floor(time);
    float anim = fract(time);
    vec2 rng = mix(hash21(index), hash21(index+1.), anim);
    pos += (rng*2.-1.)*jitter;
    
    // translate to right
    pos.x += .5;
    
    // twist it
    float angle = t;
    float radius = .1;
    pos += vec2(cos(angle),sin(angle))*radius;
    
    // fbm gyroid noise
    angle = fbm(vec3(pos,t))*6.28;
    radius = .2;
    pos += vec2(cos(angle),sin(angle))*radius;
    return pos;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    // coordinates
    vec2 uv = fragCoord/R.xy;
    vec2 p = 1.5*(fragCoord-R.xy/2.)/R.y;
    
    // scroll
    uv.x += 1./R.x;
    
    // framebuffer
    vec4 frame = texture(iChannel0, uv);
    float mask = frame.r;
    float sdf = frame.g;
    
    // interaction
    if (iMouse.z > 0.)
    {
        vec2 mouse = iMouse.xy;
        vec4 prev = texture(iChannel0, vec2(0));
        vec3 dither = hash(uvec3(fragCoord, iFrame)); 
        mouse = prev.z > 0. ? mix(mouse, prev.xy, dither.x) : mouse;
        mouse = 1.5*(mouse-R.xy/2.)/R.y;
        float thin = .04+.03*sin(iTime*20.);
        float dist = length(p-mouse);
        float msk = smoothstep(thin,.0,dist);
        if (msk > .001) frame.b = iTime;
        sdf = sdf < .001 ? dist : min(sdf, dist);
        mask += msk;
    }
    else
    {
        // accumulate noisy results
        for (float frames = 20.; frames > 0.; --frames)
        {
            // cursor timeline with noise offset
            float f = float(iFrame) + frames * 200.;
            vec3 rng = hash(uvec3(fragCoord, f));
            float cursor = rng.x*.03+iTime;

            // brush
            float thin = .04+.03*sin(cursor*20.);
            float dist = length(p-move(cursor));
            float msk = smoothstep(thin,.0,dist);

            // timestamp
            if (msk > .001) frame.b = iTime;

            // distance
            sdf = sdf < .001 ? dist : min(sdf, dist);

            // accumulate
            mask += msk;
        }
    }

    // save data
    frame.r = mask;
    frame.g = sdf;
    fragColor = frame;
    
    // avoid glitch after disabling fullscreen
    if (fragCoord.x > R.x-1.) fragColor = vec4(0,0,0,1);
    
    if (fragCoord.x < 1. && fragCoord.y < 1.)
    {
        fragColor = iMouse;
    }
}