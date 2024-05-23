//L1 particle buffer - simulation
//L2 directional neighbor graph 4x - sort

int ID;
obj O; //this object

//sort arrays
vec4 lnk0, lnk1;
vec4 d0, d1;

//L3
vec4 EA[SN.x]; //element array

void insertion_sort(float t, int id);
obj getObj(int id); vec4 saveObj(int i);
bool iscoincidenceEA(int id);
void sort0(int idtemp, int D); void sort1(int idtemp, int D);

float Kernel(float d, float h)
{
    return exp(-sqr(d/h))/(PI*sqr(h));
}

float KernelGrad(float d, float h)
{
    return 2.*d*Kernel(d,h)/sqr(h);
}

vec2 borderF(vec2 p)
{
    
    float d = min(min(p.x,p.y),min(R.x-p.x,R.y-p.y));
    return exp(-max(d,0.)*max(d,0.))*((d==p.x)?vec2(1,0):(
    		(d==p.y)?vec2(0,1):(
            (d==R.x-p.x)?vec2(-1,0):vec2(0,-1))));
}

void mainImage( out vec4 Q, in vec2 pos )
{
    //4 pix per layer, 3 layers
    sN = SN; 
    N = ivec2(R*P/vec2(sN));
    TN = N.x*N.y;
    int S = 3; //log2(sN.x)
    
    ivec2 p = ivec2(floor(pos));
    if(any(greaterThan(p, sN*N-1))) discard;
   
    ivec3 sid = xy2i(p); ID = sid.x;
    O = getObj(ID);
    d0 = vec4(1e6); d1 = vec4(1e6);
    lnk0 = vec4(-1); lnk1 = vec4(-1);
    
    switch(sid.z)
    {
    case 0: //particle
        if(sid.z >= 3) discard;
        float sk = 0.;
        
        //scale /=sk;
        vec2 F =-0.001*(O.X - R*0.5)/(pow(length(O.X - R*0.5),1.)+1.); 
        vec2 Fp = vec2(0);
        float avgP = 0.;
     
         float scale = 0.21/density; //radius of smoothing
        float Div = 0.;
        float Rho = Kernel(0., scale);
           vec2 avgV = vec2(O.V)*Rho;
        vec3 avgCol = vec3(O.Y.xyz);
        float Gsum = 1.;
        float curscale = 1e10;
        float avgSc = 0.;
        
        loop(j,4)
        {
            vec4 nb = texel(ch0, i2xy(ivec3(ID, j, 1)));
            loop(i,4)
            {
                if(nb[i] < 0. || nb[i] > float(TN)) continue;
                obj nbO = getObj(int(nb[i]));
                
               
                float d = distance(O.X, nbO.X);
                vec2 dv = (nbO.V - O.V); //delta velocity
                vec2 dx = (nbO.X - O.X); //delta position 
                vec2 ndir = dx/(d+0.001); //neighbor direction
                //SPH smoothing kernel
                float K = Kernel(d, scale);
                float dK = KernelGrad(d, scale);
               
                //Gkernel
                float G = 1./(d*d+0.01);
                float dotv = dot(ndir, dv); //divergence
                vec2 pressure = -0.5*(nbO.Pressure/sqr(nbO.Rho) + 
                                    O.Pressure/sqr(O.Rho))*ndir*K;//pressure gradient
                curscale = min(curscale, d);
                Gsum += 1.;
                Div += dotv*K; // local divergence
                Rho += K;
                avgCol += nbO.Y.xyz;
                avgP += nbO.Pressure*K;
                avgV += nbO.V*K;
                
                F += pressure + 3.*ndir*dotv*K    
                    + 0.00*ndir*exp(-0.05*density*d)*(1.-K); //surfacce tension
                Fp += ndir*(-exp(-smoothR*d/scale));
            }
        }
        
         //border conditions
        
        /*Fp += 0.5*borderF(O.X);
        F += 0.1*borderF(O.X);
       
        vec2 bdf = borderF(O.X);
        O.V -= 0.05*length(bdf)*O.V;
        
        if(R.x - O.X.x < 2.) O.V.x = -abs(O.V.x);
        if(O.X.x < 2.) O.V.x = abs(O.V.x);
        if(R.y - O.X.y < 2.) O.V.y = -abs(O.V.y);
        if(O.X.y < 2.) O.V.y = abs(O.V.y);*/
        
        
        if(iMouse.z > 0.) 
        {
            float d = distance(iMouse.xy, O.X);
            O.Y.xyz += 3.*(0.5+0.5*sin(vec3(1,2,3)*iTime))/(0.2*d*d+2.);
            F += 0.01*(iMouse.xy - iMouse.zw)/(0.2*d*d+2.);
        }
        
        O.Rho = Rho;
        O.Scale = curscale; //average distance
        O.SScale = avgSc/Gsum; //average average distance
        
        float r = 7.;
        float D = 1.;
        float waterP = 0.05*density*(pow(abs(O.Rho/density), r) - D);
        float gasP = 0.4*O.Rho;
        O.Pressure = min(waterP,0.03);
        if(iFrame > 20) O.Pressure += 0.*(avgP/O.Rho - O.Pressure);
        
        
        O.V += F*dt;
        O.V -= O.V*(0.5*tanh(8.*(length(O.V)-1.5))+0.5);
        O.X += (O.V)*dt; //advect
        
        
        
        
        
        //color diffusion
        
        O.Y.xyz = 0.999*mix(avgCol/Gsum, O.Y.xyz,0.9995)
        + 0.05*(exp(-0.1*distance(O.X,R*0.3))*sin(vec3(1,2,3)*iTime)
             + exp(-0.1*distance(O.X,R*0.7))*sin(vec3(2,3,1)*iTime));
        
        
        
        if(iFrame<10)
        {
            O.X = R*vec2(i2xy(ivec3(ID,0,0)))/vec2(N*sN);
            O.Pressure = 0.;
            O.Scale = 5.;
            O.Rho = 5.;
            O.SScale = 1.;
        }

        Q = saveObj(sid.y);
        return;
        
    case 1: //dir graph
        //sort neighbors and neighbor neighbors
        vec4 nb0 = texel(ch0, i2xy(ivec3(ID, sid.y, 1)));
        loop(i,4)
        {
            sort0(int(nb0[i]), sid.y);  //sort this
            //use a sudorandom direction of the neighbor
            vec4 nb1 = texel(ch0, i2xy(ivec3(nb0[i], (iFrame+ID)%4, 1)));
            loop(j,2)
            {
                sort0(int(nb1[j]), sid.y);  
            }
        }
        
        //random sorts
        loop(i,4) sort0(int(float(TN)*hash13(vec3(iFrame, ID, i))), sid.y);
        
        Q = lnk0;
        return;
    }
     
}

vec4 saveObj(int i)
{
    switch(i)
    {
    case 0:  
        return vec4(O.X, O.V);
    case 1:
        return vec4(O.Pressure, O.Rho, O.SScale, O.Scale);
    case 2:
        return O.Y;
    case 3:
        return vec4(0.);
    }
}

obj getObj(int id)
{
    obj o;
    
    vec4 a = texel(ch0, i2xy(ivec3(id, 0, 0))); 
    o.X = a.xy; o.V = a.zw;
    
    a = texel(ch0, i2xy(ivec3(id, 1, 0))); 
    o.Pressure = a.x;
    o.Rho = a.y;
    o.SScale = a.z;
    o.Scale = a.w;
    
    o.Y = texel(ch0, i2xy(ivec3(id, 2, 0)));
    
    o.id = id;
    return o;
}

void insertion_sort(float t, int id)
{
	if(d0.x > t)
    {
        d0 = vec4(t, d0.xyz);
        lnk0 = vec4(id, lnk0.xyz);
    }else if(d0.y > t && d0.x < t)
    {
        d0.yzw = vec3(t, d0.yz);
        lnk0.yzw = vec3(id, lnk0.yz);
    }else if(d0.z > t&& d0.y < t)
    {
        d0.zw = vec2(t, d0.z);
        lnk0.zw = vec2(id, lnk0.z);
    }else if(d0.w > t && d0.z < t)
    {
        d0.w = t;
        lnk0.w = float(id);
    }
}

bool iscoincidence(int id)
{
    return (id < 0) || 
      		(id == ID) ||
           any(equal(lnk0,vec4(id)));
}

void sort0(int idtemp, int D) //sort closest objects in sN.x directions
{
    if(iscoincidence(idtemp)) return; //particle already sorted
    
    vec2 nbX = texel(ch0, i2xy(ivec3(idtemp, 0, 0))).xy; 
   
    vec2 dx = nbX - O.X;
    int dir = int(2.*(atan(dx.y, dx.x)+PI)/PI); 
    
    if(dir != D) return; //not in this sector
    
    float t = length(dx);
   
    insertion_sort(t, idtemp);
}