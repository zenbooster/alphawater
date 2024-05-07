
// globals
float rng;
float glow;

float map(vec3 p)
{
    float dist = 100.;
    
    // travel
    p.z -= iTime * .5;
    vec3 q = p;
    
    // repeat
    float grid = 4.;
    float id = hash13(floor(p/grid-.5));
    p = repeat(p,grid);
    p += (hash31(id*100.)*2.-1.);
    
    // spice
    float noise = fbm(p);
    
    // jazz
    float t = iTime * .2 + rng * .01 + id * 196.;
    
    // kaleido
    float a = 1.;
    const float count = 5.;
    for (float i = 0.; i < count; ++i)
    {
        // funk
        float tempo = iTime*.7+i*2.+p.z*.5+q.z*.1;
    
        // move
        float rythm = pow(sin(tempo)*.5+.5, 10.);
        
        // rotation
        p.xz *= rot((rythm*.1+t)/a);
        p.yx *= rot((rythm*.2+t)/a);
        
        // fold
        float r = (rythm * .5 + .05 + noise*.1)/a;
        p.xz = abs(p.xz)-r*a;
        
        // shape
        dist = min(dist, length(p.xz));
        
        // falloff
        a /= 1.2 + 0.7 * id;
    }
    
    // shine
    glow += .0005/dist;

    return dist;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec3 color = vec3(0);
    
    // coordinates
    vec2 p = (fragCoord-iResolution.xy/2.)/iResolution.y;
    vec3 ray = normalize(vec3(p, -1.));
    vec3 pos = vec3(0,0,0) + ray;

    // globals
    rng = hash13(vec3(fragCoord, iFrame));
    glow = 0.;
    
    // raymarch
    float total = 0.;
    float maxDist = 20.;
    for (float steps = 35.; steps > 0.; --steps)
    {
        float dist = map(pos);
        if (dist < .0001 * total || total > maxDist) break;
        total += dist;
        pos += ray * dist;
    }

    // palette
    vec3 tint = .5 + .5 * cos(vec3(1,2,3)*5.5 + length(pos) * .2 - .5);
    
    fragColor = vec4(tint * glow,1.0);
}