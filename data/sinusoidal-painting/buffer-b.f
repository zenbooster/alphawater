
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{

    vec2 uv = fragCoord/iResolution.xy;
    
    // compute normal
    vec4 color = texture(iChannel0, uv);
    vec3 unit = vec3(1./iResolution.xy, 0);
    vec3 normal = normalize(vec3(
                            T(uv+unit.xz)-T(uv-unit.xz),
                            T(uv-unit.zy)-T(uv+unit.zy),
                            color.r));
                            
    fragColor = vec4(normal, 1.);
}