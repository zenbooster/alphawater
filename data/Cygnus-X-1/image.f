Main 
        
     Q = .8*atan(A(U).w*vec4(1000,100,10,1));
     for (float i = -20.; i <20.;i++) {
     
         Q.zyxw += .001*exp(-abs(i)/10.)*.8*(A(U+5.*vec2(i,0)).w*vec4(1000,100,10,1));
      Q.zyxw += .001*exp(-abs(i)/10.)*.8*(A(U+5.*vec2(0,i)).w*vec4(1000,100,10,1));
     
         
         vec2 o = U+3.*vec2(2.*i,1.*i);
         
         
         vec2 r1 = o-vec2(.5,.5)*R+vec2(-.2,0)*R.y*ei(.01*iTime);
         vec2 r2 = o-vec2(.5,.6)*R+vec2(.2,0)*R.y*ei(.01*iTime);
         float l1 = length(r1);
         //float l2 = length(r2);
        if (l1<.015*R.y) Q.xz += 2.*exp(-abs(i)/15.)*A(o).w;
        //if (l2<.01*R.y) Q.w += 1./R.y;
     }
}