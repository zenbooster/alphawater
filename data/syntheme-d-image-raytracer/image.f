//Material
struct Material
{
    float Ka;
    float Kd;
    float Ks;
    float Kn;
};
    
struct ShadeInfo
{
    vec3 shadedCol;
    float Ks;
};
    
const int MAX_NB_BOUNCES = 5;

//camera parameters
vec3 cameraPos = vec3(6,4,-5);
const vec3 cameraTarget = vec3(3,1,-8);
const float cameraFovY = 80.0;

//Sphere parameters
vec3 spherePos = cameraTarget + vec3(0,1,2);
const float sphereRadius = 1.0;
const vec3 sphereCol = vec3(1,0,0);
const Material sphereMat = Material(0.2,0.7,1.0,50.0);
const int sphereId = 1;

//Light parameters
const vec3 ambiantCol = vec3(0,0,1);
const vec3 lightCol = vec3(1,1,1);
vec3 lightPos = vec3(8,10,-12);

//Sky parameters
const vec3 skyCol = vec3(0);
const int skyId = 0;

//Plane parameters
const vec3 planePos = vec3(0, 0.1, 0);
const vec3 planeNormal = vec3(0,1.0,0);
const vec3 planeCol1 = vec3(1.0);
const vec3 planeCol2 = vec3(0.4);
const Material planeMat = Material(0.2,1.0,0.2,5.0);
const int planeId = 2;

const int PIXEL_SAMPLING_GRID_SIZE = 5;
const int PIXEL_SAMPLING_SIZE = 5;

vec2 transform_to_apply(vec2 xy){
    xy = (2.*xy.xy - iResolution.xy)/iResolution.y;
    return vec2(xy.x,xy.y);
}

float raySphere(vec3 rayPos, vec3 rayDir, vec3 spherePos, float sphereRadius, out vec3 intersecS, out vec3 normalS)
{    
    vec3 diff = rayPos - spherePos;
    
    float a = dot(rayDir,rayDir);
    float b = 2.0 * dot(diff,rayDir);
    float c = dot(diff,diff) - sphereRadius * sphereRadius;
    
    float di = b*b - 4.0*a*c;
    
    if(di >= 0.0)
    {
        float sdi = sqrt(di);
        float den = 1.0 / (2.0 * a);
        float t1 = (-b - sdi) * den;
        float t2 = (-b + sdi) * den;
        
        intersecS = rayPos + t1 * rayDir;
        normalS = normalize(intersecS - spherePos);
        
        return t1;
    }
    
    return -1.;
}

float rayPlane(vec3 rayPos, vec3 rayDir, vec3 planePos, vec3 planeNormal, out vec3 intersecPt, out vec3 normal)
{
    if(dot(rayDir,planeNormal)!=0.0)
    {
        float t = dot(planePos - rayPos,planeNormal)/dot(rayDir,planeNormal);
        
        intersecPt = rayPos + t * rayDir;
        normal = planeNormal;
        
        return t;
    }
    return -1.0;
}

void computeCameraRayFromPixel(in vec2 pixCoord, out vec3 rayPos, out vec3 rayDir)
{
    rayPos = cameraPos;
    
    float f = 1./tan(radians(cameraFovY)/2.);
    
    vec3 PT = cameraTarget - cameraPos;
    vec3 cz = normalize(PT);
    
    vec3 PO = f*cz;
    
    vec3 cyT = vec3(0,-1.,0);
    vec3 cxT = cross(cyT,cz);
    
    vec3 cx = normalize(cxT);
    
    vec3 cy = cross(cz,cx);
    
    vec3 OS = pixCoord.x * cx - pixCoord.y * cy;
    
    rayDir = normalize(PO+OS);
}

vec3 computePhongShading(vec3 sphereCol, Material sphereMat,float shadowFactor, vec3 normal, vec3 L, vec3 R, vec3 V)
{
    vec3 A = sphereMat.Ka * ambiantCol;
    vec3 D = sphereMat.Kd * sphereCol * lightCol * max(dot(L,normal),0.0);
    vec3 S = sphereMat.Ks * lightCol * pow(max(dot(R,V),0.0),sphereMat.Kn);
    return A+shadowFactor*D+shadowFactor*S;
}


float computeNearestIntersection(vec3 rayPos, vec3 rayDir, out int objectId, out vec3 intersecI, out vec3 normalI)
{
    vec3 intersecS, normalS;
    float distSphere = raySphere(rayPos, rayDir, spherePos, sphereRadius, intersecS, normalS);
    vec3 intersecP, normalP;
    float distPlane = rayPlane(rayPos, rayDir, planePos, planeNormal, intersecP, normalP);
    if(distSphere > 0.0 && distPlane > 0.0)
    {
        float distMin = min(distSphere,distPlane);
        objectId = (distSphere == distMin) ? sphereId : planeId; 
        intersecI = (distSphere == distMin) ? intersecS : intersecP; ;
        normalI = (distSphere == distMin) ? normalS : normalP; ;
        return distMin;
    }
    else
    {
        if(distSphere > 0.0)
        {
            objectId = sphereId;
            intersecI = intersecS;
            normalI = normalS;
            return distSphere;
        }
        else if (distPlane > 0.0)
        {
            objectId = planeId;
            intersecI = intersecP;
            normalI = normalP;
            return distPlane;
        }
        else
        {
           	objectId = skyId;
            return -1.0;
        }
    }
}

vec3 getSphereColorAtPoint(vec3 pt)
{
    return sphereCol;
}


vec3 getPlaneColorAtPoint(vec3 pt)
{
    return mod(floor(pt.x*0.5) + floor(pt.z*0.5),2.0) < 1.0 ? planeCol1 : planeCol2;
}


vec3 getObjectColorAtPoint(int objectId, vec3 pt, out Material objectMat)
{
    if (objectId == sphereId)
    {
        objectMat = sphereMat;
        return getSphereColorAtPoint(pt);
    }
    else if (objectId == planeId)
    {
        objectMat = planeMat;
        return getPlaneColorAtPoint(pt);
    }
        
    return skyCol;
}

float getShadowFactorAtPoint(vec3 I, vec3 N, Material objectMat, vec3 L, float Ldist)
{
    I += 0.001*N;
    int objId;
    vec3 intersecS, normalS;
    float d = computeNearestIntersection(I,L,objId,intersecS,normalS);
    return (d<=0.0 || d>=Ldist)?1.0:objectMat.Ka;
}

vec3 RaytraceAtPixelCoord(vec2 pixCoord)
{
    vec3 rayPos, rayDir;
    computeCameraRayFromPixel(pixCoord, rayPos, rayDir);
    
    ShadeInfo infos[MAX_NB_BOUNCES];
    int nbBounces = 0;
    do
    {
        int objectId;
        vec3 intersecI, normalI;
        float distI = computeNearestIntersection(rayPos,rayDir,objectId,intersecI,normalI);
        
        if(distI <= 0.0)
        {
            infos[nbBounces].shadedCol = skyCol;
            infos[nbBounces].Ks = 0.0;
            break;
        }
        
        Material objMat;
    	vec3 col = getObjectColorAtPoint(objectId, intersecI, objMat);
    	vec3 L = normalize(lightPos - intersecI);
    	vec3 R = normalize(2.0*dot(normalI,L)*normalI - L);
    	vec3 V = -rayDir;
        
        float Ldist = distance(intersecI,lightPos);
        
        float shadowFactor = getShadowFactorAtPoint(intersecI, normalI, objMat, L, Ldist);
        
    	
    	vec3 c = computePhongShading(col, objMat, shadowFactor, normalI, L, R, V);
        
        infos[nbBounces].shadedCol = c;
        infos[nbBounces].Ks = objMat.Ks;
        
        rayPos = intersecI + 0.001*normalI;
        rayDir = normalize(2.0*dot(normalI,V)*normalI - V);
       
        
        nbBounces++;
        
    }while(nbBounces < MAX_NB_BOUNCES);
    
    vec3 resCol = vec3(0);
    
   	do
    {
        resCol = infos[nbBounces].shadedCol + infos[nbBounces].Ks * resCol;
        nbBounces--;
    }while(nbBounces >= 0);
    
    return resCol;
}


vec2 noise2(vec2 location, vec2 delta)
{
    const vec2 c = vec2(12.9898,78.233);
    const float m = 43758.5453;
    return vec2(
        fract(sin(dot(location + delta, c)) * m),
        fract(sin(dot(location + vec2(delta.y,delta.x),c)) * m)
        );
}

void animateScene(float time)
{
    const float pi = 3.1415926535;
    const float rs = 2.0;
    const float spr = 5.0;
    float as = 2.0 * pi * time / spr;
    
    spherePos = cameraTarget + rs * vec3(-sin(as),0.0,cos(as)) + vec3(0,1,0);
    
    lightPos += vec3(0,10.5 + 9.5 * cos(time) - 10.,0);
    
    float targetDist = length(cameraTarget - cameraPos);
    cameraPos -= vec3(0,0,targetDist);
    cameraPos += targetDist * vec3(sin(time),max(sin(time*0.5),0.),cos(time));
}


void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    
    float time = iTime;
    
    animateScene(time*1.0);
    
   
    fragCoord = transform_to_apply(fragCoord.xy);
    
    vec3 resCol = vec3(0);
    
    for (int i=0; i<PIXEL_SAMPLING_GRID_SIZE; i++) 
    {
        for (int j=0; j<PIXEL_SAMPLING_SIZE; j++) 
        {
            vec2 offset = vec2(float(i)/(float(PIXEL_SAMPLING_GRID_SIZE)*iResolution.x),float(j)/(float(PIXEL_SAMPLING_SIZE)*iResolution.y));
          
			resCol += RaytraceAtPixelCoord(fragCoord + offset);
      	}
    }
    resCol /= float(PIXEL_SAMPLING_GRID_SIZE * PIXEL_SAMPLING_SIZE);
    
    fragColor = vec4(resCol,1);
}