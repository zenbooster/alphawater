void mainImage( out vec4 U, in vec2 pos )
{
    vec2 uv = pos/R;
    ivec2 p = ivec2(pos);
        
    vec4 data = T(pos); 
    vec2 X = DECODE(data.x) + pos;
    vec2 V = DECODE(data.y);
    float M = data.z;
    
    if(M != 0.) //not vacuum
    {
        //Compute the force
        vec2 F = vec2(0.);
        
        //get neighbor data
        vec4 d_u = T(pos + dx.xy), d_d = T(pos - dx.xy);
        vec4 d_r = T(pos + dx.yx), d_l = T(pos - dx.yx);
        
        //position deltas
        vec2 p_u = DECODE(d_u.x), p_d = DECODE(d_d.x);
        vec2 p_r = DECODE(d_r.x), p_l = DECODE(d_l.x);
        
        //velocities
        vec2 v_u = DECODE(d_u.y), v_d = DECODE(d_d.y);
        vec2 v_r = DECODE(d_r.y), v_l = DECODE(d_l.y);
       
        
        
        //pressure gradient
        vec2 p = vec2(Pressure(d_r) - Pressure(d_l),
                      Pressure(d_u) - Pressure(d_d));
        
        //density gradient
        vec2 dgrad = vec2(d_r.z - d_l.z,
                      d_u.z - d_d.z);
        
        //velocity operators
        float div = v_r.x - v_l.x + v_u.y - v_d.y;
        float curl = v_r.y - v_l.y - v_u.x + v_d.x;
        //vec2 laplacian = 
            
        F -= 0.6*M*dgrad;
        

        float ang = atan(V.y, V.x);
        float dang =sense_ang*PI/float(sense_num);
        vec2 slimeF = vec2(0.);
        //slime mold sensors
        range(i, -sense_num, sense_num)
        {
            float cang = ang + float(i) * dang;
        	vec2 dir = (1. + sense_dis*pow(max(M - 0., 0.), distance_scale))*Dir(cang);
        	vec3 s0 = C(X + dir).xyz;  
   			float fs = pow(s0.z, force_scale);
            float os = oscil_scale*pow(s0.z - M, oscil_pow);
        	slimeF +=  sense_oscil*Rot(os)*s0.xy 
                     + sense_force*Dir(ang + sign(float(i))*PI*0.5)*fs; 
        }
        
        //remove acceleration component and leave rotation
        slimeF -= 1.*dot(slimeF, normalize(V))*normalize(V);
		F += slimeF/float(2*sense_num);
        
        if(iMouse.z > 0.)
        {
            vec2 dx= pos - iMouse.xy;
             F += 0.1*Rot(PI*0.5)*dx*GS(dx/30.);
        }
        
        //integrate velocity
        V += Rot(-0.*curl)*F*dt/M;
        
        //acceleration for fun effects
        V *= 1. + acceleration;
        
        //velocity limit
        float v = length(V);
        V /= (v > 1.)?v/1.:1.;
    }
    
    //mass decay
   // M *= 0.999;
    
    //input
    //if(iMouse.z > 0.)
    //\\	M = mix(M, 0.5, GS((pos - iMouse.xy)/13.));
    //else
     //   M = mix(M, 0.5, GS((pos - R*0.5)/13.));
    
    //save
    X = clamp(X - pos, vec2(-0.5), vec2(0.5));
    U = vec4(ENCODE(X), ENCODE(V), M, 0.);
}