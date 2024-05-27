void mainImage( out vec4 O, vec2 u )
{   ivec2 U = ivec2( u / iResolution.xy * 8. ) % 2;
    O = vec4( U.x == U.y );
}