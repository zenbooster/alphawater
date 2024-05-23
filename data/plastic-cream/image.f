
// Plastic Cream

// i was playing with Conway's game of life cellular automaton
// thinking i had an intuition for a smooth fading version
// with a lifetime gradient and a neighbor ratio

// found by accident that it can make reaction diffusion patterns
// got overwhelmed but couldn't posted another turing pattern feedback
// so i went a bit too far and now it's weird enough as i like it

// iteration from Wasp Blanket https://www.shadertoy.com/view/NlVBz1

#define T(uv) abs(texture(iChannel0,uv).b)
#define N(v) normalize(v)

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 uv = fragCoord/iResolution.xy;
    vec4 frame = texture(iChannel0, uv);
    
    // tints
    vec3 tint = .5 + .5 * cos(vec3(1,2,3)*5. + length(uv-.5)*3.);
    fragColor = vec4(frame.r * tint, 1);
    tint = .5 + .5 * cos(vec3(1,2,3)*5. + length(uv-.5)*2. + 3.);
    fragColor.rgb += frame.g * tint;
    
    // normal
    float height = 1.;
    vec3 unit = vec3(20./iResolution.xy, 0);
    vec3 normal = normalize(vec3(T(uv+unit.xz)-T(uv-unit.xz),
                                 T(uv-unit.zy)-T(uv+unit.zy),
                                 T(uv) * height));
    
    // light
    fragColor += vec4(.5) * clamp(dot(normal, N(vec3(-1,4,1))), 0., 1.);
    fragColor += vec4(.5) * pow(clamp(dot(normal, N(vec3(-1,1,2))), 0., 1.), 20.);
    
    // shadows
    fragColor -= vec4(0.5) * clamp(dot(normal, N(vec3(0,-4,1))), 0., 1.);
    fragColor *= pow(max(abs(frame.b)-.02,.0), .5);
}