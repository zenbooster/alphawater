Main 
    Q = vec4(0);
        vec2 V = vec2(0);
        for (float x = -3.;x <= 3.; x++)
        for (float y = -3.;y <= 3.; y++) {

            vec2 u = vec2(x,y);

            vec4 q = A(U+u);
            vec2 v = unpackSnorm2x16(uint(q.z));
            float k = 1.1;
            vec4 o = clamp(v.xyxy+q.xyxy+u.xyxy+k*vec4(-.5,-.5,.5,.5),-.5,.5);
            float w = (o.z-o.x)*(o.w-o.y)/k/k;
            v.xy = 0.5*(o.xy+o.zw);
            V.xy += v.xy*q.w*w;
            Q.xy += q.xy*q.w*w;
            Q.w += q.w*w;
        }
        if (Q.w>0.) {
            Q.xy/=Q.w;
            V /= Q.w;
        }
        
        Q.z = float(packSnorm2x16(V.xy));
        
}