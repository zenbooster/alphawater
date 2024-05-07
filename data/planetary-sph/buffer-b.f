//voronoi particle tracking + graph augmented

vec2 sc(vec2 p)
{
    return SC*(p - 0.5*R) + 0.5*R;
}

float d;
int id;
vec2 p;

float particleDistance(int i)
{
    return distance(p, sc(texel(ch0, i2xy(ivec3(i, 0, 0))).xy));
}

void sort(int utemp)
{
    if(utemp < 0) return; 
   	float dtemp = particleDistance(utemp);
    if(dtemp < d) //sorting
    {
        d = dtemp;
        id = utemp;
    }
}


void mainImage( out vec4 Q, in vec2 pos )
{
    sN = SN; 
    N = ivec2(R*P/vec2(sN));
    TN = N.x*N.y;
    d = 1e10;
    id = 1;
    p = pos;
    ivec2 pi = ivec2(floor(pos));
    
    sort(1+0*int(texel(ch1, pi).x));
    
    int ID = id;
    loop(j,8)
    {
        
        int nbid = int(texel(ch1, pi+cross_distribution(j)).x);
        sort(nbid);
    }
    
    loop(j,4)
    {
        vec4 nb = texel(ch0, i2xy(ivec3(ID, j, 1)));
        loop(i,4)
    	{ 
            sort(int(nb[i]));  //sort this
        }
    }
    
    loop(i,4) //random sort
    {
        sort(int(float(TN)*hash13(vec3(iFrame, pi.x, pi.y*i))));
    }
    
   	Q = vec4(id, d, 0, 0);
}