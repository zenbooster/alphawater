//from https://www.shadertoy.com/view/dtGGzK

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec4 col = vec4(0);
    if (floor(fragCoord) == vec2(1,0)) {
        vec2 prevpos = texelFetch(iChannel0, ivec2(1,0), 0).zw; //previous dragged mouse pos
        vec4 mp = texelFetch(iChannel0, ivec2(2,0), 0); //prev mouse pos
        vec4 m = iMouse; //current mouse pos
        
        //if mouse button up
        if (m.z < 0.0 && mp.z >= 0.0){
            col = texelFetch(iChannel0, ivec2(1,0), 0).xyxy;
        } else if (m.z >= 0.0){
            col = vec4(m.xy - abs(m.zw) + prevpos, prevpos);
        } else {
            col = texelFetch(iChannel0, ivec2(1,0), 0);
        }
    }
    //the last mouse position
    if (floor(fragCoord) == vec2(2,0)) {
        col = iMouse;
    }
    fragColor = col;
}