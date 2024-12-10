void mainImage( out vec4 U, in vec2 pos )
{
    ivec2 p = ivec2(pos);
    
    vec2 X = vec2(0);
    vec2 V = vec2(0);
    float M = 0.;
    
    //basically integral over all updated neighbor distributions
    //that fall inside of this pixel
    //this makes the tracking conservative
    range(i, -2, 2) range(j, -2, 2)
    {
        vec2 tpos = pos + vec2(i,j);
        vec4 data = T(tpos);
       
        vec2 X0 = DECODE(data.x) + tpos;
    	vec2 V0 = DECODE(data.y);
    	vec2 M0 = data.zw;
       
        X0 += V0*dt; //integrate position

        //particle distribution size
        float K = distribution_size;
        
        vec4 aabbX = vec4(max(pos - 0.5, X0 - K*0.5), min(pos + 0.5, X0 + K*0.5)); //overlap aabb
        vec2 center = 0.5*(aabbX.xy + aabbX.zw); //center of mass
        vec2 size = max(aabbX.zw - aabbX.xy, 0.); //only positive
        
        //the deposited mass into this cell
        float m = M0.x*size.x*size.y/(K*K); 
        
        //add weighted by mass
        X += center*m;
        V += V0*m;
      
        //add mass
        M += m;
    }
    
    //normalization
    if(M != 0.)
    {
        X /= M;
        V /= M;
    }
    
    //mass renormalization
    float prevM = M;
    M = mix(M, 0.1, 0.1);
    V = V*prevM/M;
    
    //initial condition
    if(iFrame < 1)
    {
        X = pos;
        vec2 dx0 = (pos - R*0.3); vec2 dx1 = (pos - R*0.7);
        V = 0.5*Rot(PI*0.5)*dx0*GS(dx0/30.) - 0.5*Rot(PI*0.5)*dx1*GS(dx1/30.);
        V += 0.2*Dir(2.*PI*hash11(floor(pos.x/20.) + R.x*floor(pos.y/20.)));
        M = 0.1 + pos.x/R.x*0.01 + pos.y/R.x*0.01;
    }
    
    X = clamp(X - pos, vec2(-0.5), vec2(0.5));
    U = vec4(ENCODE(X), ENCODE(V), M, 0.);
}