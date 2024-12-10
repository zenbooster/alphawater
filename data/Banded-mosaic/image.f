

vec2 rotate(vec2 v, float a) {
	float s = sin(a);
	float c = cos(a);
	mat2 m = mat2(c, -s, s, c);
	return m * v;
}

vec2 triangle_wave(vec2 a,float num){
    //a = rotate(a,num*radians(180.));
    //a += .5;
    vec2 to_return = abs(fract((a+vec2(1.,0.5))*1.5)-.5);
    //to_return /= 1.5; //makes another interesting pattern
    return
        to_return
        //to_return + dot(to_return,to_return)/8. //makes another interesting pattern

    ;
    //return abs(fract((a+vec2(1.,1.5)+num)*scale)-.5);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    fragColor = vec4(0.0);
    vec3 col = vec3(0.);
    float t1 = 64.;
    vec2 draggedPos = texelFetch(iChannel0, ivec2(1,0), 0).xy;
    vec2 uv = (fragCoord)/iResolution.y/t1/2.0;
    uv += vec2(iTime/2.0,iTime/3.0)/t1/8.0;
    uv -= draggedPos/iResolution.y/t1;
    float scale = 1.5;
    float offset = 0.;
    float offset1 = iTime/1000.;
        vec2 t2 = vec2(0.);
        vec2 t3 = vec2(0.);
        for(int k = 0; k < 12; k++){
            //k += int(uv.y<uv.x);
            
            //uv -= fpow(uv.x,2.); //flower pattern

            //uv = (fract(vec2(uv+vec2(.5,1.5))*scale)-.5)/scale;
            
            //A really cool pattern:
            //uv += floor(uv/1.5)/1.5;

            uv.y += fmod1(uv.y,1.5); //lace pattern
            //uv += floor(uv+vec2(1.,.5))/1.5;
            float p1 = sign(uv.x);
            //uv += ceil(uv.x)/2.;
            //uv.x -= floor(uv.x)/scale;
            //t2 = abs(t2*2.);
            
            //uv += floor(t2.x-t2.y);
            
            uv =
                //fract(uv + t2+.5)
                -abs(uv + t2)
            ;
            //uv += (distance(floor(uv),round(uv+.5)));
            //uv = max(-uv,uv*sign(uv.yx-uv));
            //uv = abs(uv);

            //uv += t2-float(k)/(4.-(uv.x-uv.y)/(4.-(uv.x-uv.y))); //this makes an even crazier pattern
            
            //uv.y += fpow(uv.y,2.)/2.;
            //uv.x += fpow(uv.x+1.,2.)/2.;
            
            uv /= scale;
            //uv -= ceil(t2.x*t2.y+5.)/4.; //mosaic pattern

            //uv += vec2(1.); //this also makes an interesting pattern
            float num = (1.+(uv.x-uv.y)/8.)*float(k)/(7.);
            
            //uv = uv*sign(uv.y-uv.x) + floor(uv.x-uv.y)*floor(t2.y-t2.x);
            //uv = abs(uv);
            
            //if(uv.y>uv.x) uv = -uv.yx; else uv *= -sign(uv-uv.yx);
            t2 =
                -p1*triangle_wave(uv-.5,num)
                //-p1*triangle_wave(abs(uv-.5),num)
                //-p1*triangle_wave(uv-.5,num)/(.5+fmod(t2.x,2.))
                //-p1*triangle_wave(uv-.5-fmod(t2.x/1.5,2.),num)
            ;
            t3 =
                p1*triangle_wave(uv.yx,num)
                //p1*triangle_wave(abs(uv.yx),num)
            ;
            //t2 -= abs(t2-.5)/8.;
            //t3 -= abs(t3-.5)/8.;
            
            
            
            uv = t2-t3;
            //uv.y -= .5*sign(uv.y);
            
            //if(uv.x > uv.y) uv /= 1.5;

            //col.x = abs(col.x-.5);
            
            //uv.x -= .5;
            
            //if(uv.x>uv.y)
            col.x =
                //max(floor((uv.y-uv.x*p1)*8.)/8.-col.x,col.x*2.25)
                max(uv.y-uv.x*p1-col.x,col.x*2.25)
            ;
            col = abs(col.yzx-vec3(1.5-col.x))/2.;
                            //if(uv.x < uv.y) col = col.yzx;
            //if(uv.x>uv.y||t2.x>t2.y) {uv=uv.yx;t2=t2.yx;}
            
            //uv.x += .5; //pink flower pattern
            //col *= col.yzx;
            //if(uv.x>uv.y) uv *= 2.; else uv -= .5;
            //if(uv.x<uv.y) uv.y += .5;

        }

    fragColor = vec4(col*2.,1.0);
}
