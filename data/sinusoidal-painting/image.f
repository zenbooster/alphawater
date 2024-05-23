
// Sinusoidal Painting
// when you let sine paint

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec4 color = vec4(0, 0, 0, 1);
    vec2 uv = fragCoord/iResolution.xy;
    vec4 data = texture(iChannel0, uv);
    float mask = data.r;
    
    if (mask > .001)
    {
        // lighting
        vec3 normal = texture(iChannel1, uv).rgb;
        vec3 light = normalize(vec3(0,1,1));
        float timestamp = data.b;
        float shade = dot(normal, light)*.5+.5;
        vec3 palette = .5+.5*cos(vec3(1,2,3)*5.+timestamp*3.);
        color.rgb = palette * shade;
        color += pow(shade,  50.);

    }
    else
    {
        // background
        color.rgb = vec3(1) * smoothstep(2., -2., length(uv-.5));

        // shadow
        float sdf = data.g;
        color *= smoothstep(-.3, .2,  sdf);
    }
    
    fragColor = color;
}