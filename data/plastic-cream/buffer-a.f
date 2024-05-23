
const float speed = 1.;
const float turbulences = 1.;
const float attraction = 2.0;

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    if (iFrame < 1)
    {
        fragColor = vec4(hash23(vec3(fragCoord, 0.)), 0, 0);
        return;
    }

    // coordinates
    vec2 uv = fragCoord/R;
    vec2 mouse = (iMouse.xy - R.xy / 2.)/R.y;
    vec2 p = (fragCoord.xy - R.xy / 2.)/R.y;
    vec2 offset = vec2(0);
    float dist = length(p);
    
    // turbulences
    float noise = fbm(vec3(p * 3., dist-iTime*.1*speed));
    noise = pow(abs(noise), 0.5);
    float angle = noise * 6.28;
    offset += turbulences * vec2(cos(angle), sin(angle));

    // attraction
    offset += attraction * normalize(p) * sin(dist * 9. + iTime);
    
    float dt = 30. * iTimeDelta;
    
    // displace frame buffer
    vec4 frame = texture(iChannel0, uv + dt * offset * speed / R);
    
    // edge spawn
    bool spawn = fragCoord.x < 1. || fragCoord.x > R.x-1.
              || fragCoord.y < 1. || fragCoord.y > R.y-1.
              || (iMouse.z > .0 && length(p-mouse) < 50./R.y);
    
    // spawn from noise
    vec2 rng = hash23(vec3(fragCoord, iFrame));
    if (spawn) frame = vec4(step(0.5, rng.x),step(0.5, rng.y),0,0);
    
    // neighbor values
    vec2 neighbors = vec2(0);
    for (float x = -1.; x <= 1.; ++x)
    {
        for (float y = -1.; y <= 1.; ++y)
        {
            if (x == 0. && y == 0.) continue;
            neighbors += texture(iChannel0, uv+vec2(x,y)/R).rg;
        }
    }
    
    // animation fade
    frame.r += 4.0 * (neighbors.r > 4.0 ? 1. : -1.) * iTimeDelta;
    frame.g += 4.0 * (neighbors.g > 4.0 ? 1. : -1.) * iTimeDelta;
    
    fragColor = vec4(clamp(frame.rg, 0., 1.), noise, 1.);//, frame.a + (neighbors.r + neighbors.g) * iTimeDelta * .1);
}