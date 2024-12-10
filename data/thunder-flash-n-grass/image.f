#define MAX_STEPS 200
#define MAX_DIST 1200.
#define SURF_DIST .1


mat2 rot(float r){
    return mat2(cos(r), sin(r), -sin(r), cos(r));
}


float random(float x)
{
    return fract(sin(x*323.9877));
}

float random (in vec2 _st) {
    return fract(
                 sin( 
                      dot(
                          _st.xy,vec2(12.9898,78.233)
                         )
                     )*
        43758.5453123);
}

vec2 random2(vec2 _st)
{
   vec2 res = vec2(fract(
                 sin( 
                      dot(
                          _st.xy,vec2(12.9898,78.233)
                         )
                     )*
        43758.5453123),
        fract(
                 sin( 
                      dot(
                          _st.xy,vec2(45.9898,31.233)
                         )
                     )*
        4358.5453123));
   res-=.5;
   return res;
}


float noise (in vec2 _st) {
    vec2 i = floor(_st);
    vec2 f = fract(_st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}

#define NUM_OCTAVES 5

float fbm ( in vec2 _st) {
    float v = 0.0;
    float a = 0.5;
    vec2 shift = vec2(100.0);
    // Rotate to reduce axial bias
    mat2 rot = mat2(cos(0.5), sin(0.5),
                    -sin(0.5), cos(0.50));
    for (int i = 0; i < NUM_OCTAVES; ++i) {
        v += a * noise(_st);
        _st = rot * _st * 2.0 + shift;
        a *= 0.5;
    }
    return v;
}
const mat2 m2 = mat2(  0.80,  0.60,
           -0.60,  0.80 );
           
float fbm_6(vec2 x)
{
    float f = 2.0;
    float s = 0.5;
    float a = 0.0;
    float b = 0.5;
    for( int i=0; i<6; i++ )
    {
        float n = noise(x);
        a += b*n;
        b *= s;
        x = f*m2*x;
    }
    
	return a;
}
float skydist(vec3 p)
{
     float h=fbm_6(p.xz/120.-iTime*0.2)*80.+550.;
    
     return h-p.y; 
}
float terraindist(vec3 p)
{
   float orih=120.*fbm_6(p.xz/100.)+200.;
  
 
   return (p.y-orih);
  
}

float RayMarchSky(vec3 ro, vec3 rd) {
	float dO=0.;
    
    for(int i=0; i<MAX_STEPS; i++) {
    	vec3 p = ro + rd*dO;
        
        float ds=(skydist(p));
       
        
        dO+=ds;
       
        if(dO>MAX_DIST||abs(ds)<SURF_DIST){break;}
       
    }
    
    return dO;
}
float RayMarchMountain(vec3 ro, vec3 rd) {
	float dO=0.;
   
    for(int i=0; i<MAX_STEPS; i++) {
    	vec3 p = ro + rd*dO;
        float ds=(terraindist(p));
        dO+=ds;       
        if(dO>MAX_DIST||abs(ds)<SURF_DIST){break;}
       
    }
    
    return dO;
}


vec3 skyrender(vec2 st,vec3 rd){
     vec2 q = vec2(0.);
    q.x = fbm( st + 0.00*iTime);
    q.y = fbm( st + vec2(1.0));
    vec2 r = vec2(0.);
    r.x = fbm( st + 1.0*q + vec2(1.7,9.2)+ 0.5*iTime );
    r.y = fbm( st + 1.0*q + vec2(8.3,2.8)+ 0.26*iTime);
     float f = fbm(st+r);
    
     vec3 col= vec3(0.22,0.28,0.41) - rd.y*1.4;
      col = mix( col, vec3(1.0), 0.52*f );
    
      col+=smoothstep(0.075,0.0,abs(f-0.79))*.46*vec3(0.54,0.52,0.67);
   
    return col;
    
}

float dseg( vec2 ba, vec2 pa )
{
	
	float h = clamp( dot(pa,ba)/dot(ba,ba),0., 1.6 );	
	return length( pa - ba*h );
}
float arc2(vec2 x,vec2 r, vec2 dir,float stepnum)
{
   
    float d=10.;
    for (float i = 0.; i < stepnum; i++)
    {
        vec2 s= normalize(random2(r)+dir);
      
        d=min(d,dseg(s,x-r));
        r += s;
      
    }
    return d;
    
}
float arc(vec2 x,vec2 r, vec2 dir,float stepnum)
{
   
    float d=10.;
    for (float i = 1.; i < stepnum; i++)
    {
        vec2 s= normalize(random2(r)+dir);
      
        d=min(d,dseg(s,x-r));
        r += s;
        
        if(mod(i,6.)==0.)  d=min(d,arc2(x,r,vec2(0.25,-0.8),fract(iTime*1.4)*5.));
      
    }
    return d*4.;
    
}
float  perlight(vec2 uv,float scale,float stepnum)
{
    float t=random(iTime*0.01);
    if(t>0.32)
        return 0.;
    vec2 or=vec2(0.03,-0.043);
    or=or+.01*sin(or+.3*iTime);
   
   
     uv.y-=0.5;
     
      float res=exp(-1.8*length(uv))*0.7;
      if(t>0.06)
      return res*0.25;
     
  
     uv*=scale;
     
   
   uv.x+=fbm_6(-uv*2.4)*0.4;
   uv.y+=fbm(uv*2.2)*.6;
    
    
    float d=100.;
    
    vec2 r=or*scale;
    
  
    
    for(float i=0.;i<=stepnum;i++)
    {
         
       
         vec2 ab=normalize(random2(r)-vec2(0,0.5+fract(iTime*0.0000006)))*0.8;
         
         d=min(d, dseg(ab,uv-r));
         r+=ab;
        
        
         
        if(mod(i,6.)==floor(random(iTime*0.00001)*2.)+1.)  d=min(d,arc(uv,r,vec2(0.65,-0.5),fract(iTime*1.4)*random(i)*12.));
        if(mod(i,8.)==floor(random(iTime*0.00002)*1.)+1.)  d=min(d,arc(uv,r,vec2(-0.67,-.5),fract(iTime*1.4)*random(i*3.)*12.));
     
        
    }
    return max(res,exp(-scale*0.15*d));
   
}
float light(vec2 uv)
{
   
    float s=0.;
   
   
    s+=perlight(uv,32.,fract(iTime*1.4)*26.);
  
    
    
    return s;
}
//Grass function shamely copied from https://www.shadertoy.com/view/lslGR8
vec4 grass(vec2 p, float x)
{
	float s = mix(0.7, 2.0, 0.5 + sin(x * 12.0) * 0.5);
	p.x += pow(1.0 + p.y, 2.0) * 0.1 * cos(x * 0.5 + iTime);
	p.x *= s;
	p.y = (1.0 + p.y) * s - 1.0;
	float m = 1.0 - smoothstep(0.0, clamp(1.0 - p.y * 1.5, 0.01, 0.6) * 0.2 * s, pow(abs(p.x) * 19.0, 1.5) + p.y - 0.6);
	return vec4(mix(vec3(0.05, 0.1, 0.0) * 0.8, vec3(0.0, 0.3, 0.0), (p.y + 1.0) * 0.5 + abs(p.x)), m * smoothstep(-1.0, -0.9, p.y));
  
}
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    
    vec2 uv = (fragCoord-.5*iResolution.xy)/iResolution.y;
    vec2 ouv=uv;
    vec3 col = vec3(0.);
     col=vec3(81./255.,89./255.,98./255.);
     col*=0.8;
   
   
    float light=light(uv);
     col+=light;
   
    
    vec3 ro=vec3(0,500.,-100.),
         lookat=vec3(0,400,1600),
         f=normalize(lookat-ro);
   
    float zoom=1.;
    
    vec3 r=cross(vec3(0,1,0),f),
         u=cross(f,r);
   
    vec3 c=ro+f*zoom,
         i=c+uv.x*r+uv.y*u,
         rd=normalize(i-ro);
   
   float d=RayMarchSky(ro,rd);
   if(d<MAX_DIST){
       vec3 p = ro + rd * d;
       
       vec3 skyf=skyrender(p.xz/100.,rd);
     
        col=mix(col,skyf,rd.y*3.);
       
      
  }
  else
  {
  d=RayMarchMountain(ro,rd);
  if(d<MAX_DIST){
        vec3 p = ro + rd * d;
        col=vec3(40./255.,49./255.,58./255.);
       
        if(p.z>800.)
        col=vec3(42./255.,53./255.,64./255.);
       
  }
  else{
   
   
   float dx=fbm(uv*vec2(6.6,2.56))*0.34*smoothstep(-0.7,0.2,uv.x);

   float dy=fbm(uv*vec2(1.6,16.2*(2.-2.*uv.x)))*1.16*smoothstep(0.,0.26,uv.y+0.45)*smoothstep(-0.2,0.5,uv.x);
  
   dy=exp(9.*(dy-1.0));
   col+=dy*vec3(0.65,0.64,0.52);
   col+=dx*vec3(0.65,0.61,0.52);
   col-=0.05;
   
  }
  }

  
     c=ro+f*8.,
     i=c+uv.x*r+uv.y*u,
     rd=normalize(i-ro);
     
     int BLADES=200;
    
     vec3 grasscol=col;
     for(int i = 0; i < BLADES; i += 1)
	 {
		float z = -(float(BLADES - i) );
        z*=10.*0.08;
		
		vec2 tc =ro.xy + rd.xy * (-z);
        
		
		tc.x += cos(float(i) * 3.0) * 4.0;
		
		float cell = floor(tc.x);
        
		
		tc.x = (tc.x - cell)-.5;
        tc.y=tc.y-498.5+(-z)*0.09;
        
     
		
		vec4 c = grass(tc, float(i) + cell * 10.0);
        
	
        grasscol=mix(grasscol,c.rgb,c.w);
        
		
	 }
  
       col=mix(grasscol,col,smoothstep(0.0,0.2,uv.y+0.42));
    
       col = pow(col * 1.2, vec3(0.8));
    
   
  
    float frog=clamp(smoothstep(0.1,0.6,fbm_6(uv*2.+vec2(-iTime*0.2,iTime*0.1))),0.,0.4);
    frog=fbm_6(uv*2.+vec2(-iTime*0.2,iTime*0.1));
    frog=fbm(uv*2.+frog-vec2(iTime*0.1,-iTime*0.12))*0.5;
    frog=smoothstep(0.0,0.9,frog);
    frog*=1.-abs(uv.y);
    col*=1.-frog;
  
    

   
    fragColor = vec4(col,1.0);
}