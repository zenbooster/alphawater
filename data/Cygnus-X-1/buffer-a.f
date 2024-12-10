Main 
         Q = A(U);
         vec2 V = unpackSnorm2x16(uint(Q.z));
         
         for (float x = -2.;x <= 2.;x++)
         for (float y = -2.;y <= 2.;y++) {
             vec2 u = vec2(x,y);
             vec4 q = A(U+u);
             float l = length(u);
             if (l>0.) {
                 float r = exp(-.2*dot(u,u))/length(u);
                Q.xy  -= .1*q.w*u*r;
             }
         }
         
         
         vec2 r1 = U-vec2(.5,.5)*R+vec2(-.2,0)*R.y*ei(.01*iTime);
         vec2 r2 = U-vec2(.5,.6)*R+vec2(.2,0)*R.y*ei(.01*iTime);
         float l1 = length(r1);
         float l2 = length(r2);
        Q.xy -= .25*(r1/l1/l1+r2/l2/l2);
        if (l1<.01*R.y) Q *= 0.99;
        if (l2<.01*R.y) Q.w += 1./R.y;
    if (iFrame < 1) {
        Q.w = 1./R.y;
        
        Q.xy = (r1/l1/l1-r2/l2/l2).yx*vec2(-1,1);
     }
         
    if (U.y< 1.||R.x-U.x<1.||U.x< 1.||R.y-U.y<1.) Q.w *= 0.;
        
        Q.z = float(packSnorm2x16(V));
    


}