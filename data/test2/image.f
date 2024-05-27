void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = fragCoord/iResolution.xy;
    vec2 uv0 = uv;
    uv0.y *= 0.5;

    vec2 uv1 = uv;
    //uv1.y *= 0.5;
    //uv1.y += 0.5;

    // Time varying pixel color
    vec3 col = (uv.y > 0.5) ? texture(iChannel0, uv0).rgb : texture(iChannel1, uv1).rgb;
    //vec3 col = (uv.y > 0.5) ? texture(iChannel0, uv0).rgb : vec3(0);

    // Output to screen
    fragColor = vec4(col,1.0);
}