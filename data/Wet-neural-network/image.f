mat2 rotate2D(float r) {
    return mat2(cos(r), sin(r), -sin(r), cos(r));
}



// based on the follow tweet:
// https://twitter.com/zozuar/status/1625182758745128981
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = (fragCoord-.5*iResolution.xy)/iResolution.y;
    vec3 col = vec3(0);
    float t = iTime;
    
    vec2 n = vec2(0),q;
    vec2 N = vec2(0);
    vec2 p = uv + t/10.;
    float S = 10.;
    mat2 m = rotate2D(1.);

    for(float j=0.;j++<30.;){
      p*=m;
      n*=m;
      q=p*S+j+n+t;
      n+=sin(q);
      N+=cos(q)/S;
      S*=1.2;
    }
    col = vec3(1, 2, 4) * ((N.x + N.y + .15)+.005/length(N));
    
    //col=pow(max(vec3(0),(N.x+N.y+.5)*.1*vec3(1,8,2)+.003/length(N)),vec3(.45));
    
    
    // Output to screen
    fragColor = vec4(col,1.0);
}