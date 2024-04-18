#include <windows.h>
//#include <scrnsave.h>
#include <iostream>
#include <string>
#include <vector>
#include <io.h>
#include <fcntl.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wexpansion-to-defined"
#include <glm/glm.hpp>
#pragma GCC diagnostic pop

#include <GL/gl.h>

using namespace std;

const char *vertexShaderSource = 
R"(#version 330 core
layout (location = 0) in vec2 position;            
layout (location = 1) in vec2 inTexCoord;

out vec2 texCoord;
void main(){
    texCoord = inTexCoord;
    gl_Position = vec4(position.x, position.y, 0.0f, 1.0f);
})";

const char *fragmentShaderSource = 
R"(#version 330 core
in vec2 texCoord;
uniform vec2      iResolution;
uniform float     iTime;
out vec4 fragColor;

/*float M(inout vec3 s, inout vec3 q, float t)
{
  vec4 v = vec4(0, 33, 55,0);
  vec4 z = cos( v + t*.4);
  mat2 m0 = mat2(z.x, z.y, z.z, z.w);
  z = cos( v + t*.3);
  mat2 m1 = mat2(z.x, z.y, z.z, z.w);

  s.xz *= m0;
  s.xy *= m1;
  float a = sin(q+sin(q+sin( q ).y).z).x*.5 - 1.;
  q = s + s + t;
  return length(s+sin(t*.7)) * log(length(s)+1.) + a;
}

void mainImage(out vec4 o, vec2 u)
{
    float z = 0.0;
    o *= 0.;
    vec3 q, p, s;
    vec2 R = iResolution.xy;
    float t = mod(iTime, 251.328);
    float d = 2.5;
    float r;
    for(; z < 7.; z++ )
    {
        s = p = vec3((u - .5 * R.xy) / R.y * d, 5. - d);
        r = M(s, q, t);
        d += min(r, 1.);
        s = p + .1;
        o += max(.7-r*.28 ,0.);
        o *= vec4(.1, .3, .4,0) - vec4(10, 5, 6,0) * (M(s, q, t) - r) / 4.;
    }
    o.a = 1.0;
}*/


/*

	Fractal Flythrough
	------------------

	Moving a camera through a fractal object. It's a work in progress.

	I was looking at one of Dr2's shaders that involved moving a camera through a set of way points (set
	out on the XZ plane), and thought it'd be cool to do a similar 3D version. The idea was to create a
	repetitive kind of fractal object, give the open space nodes a set random direction, create some 
	spline points, then run a smooth camera through them. Simple... right? It always seems simple in my
	head, but gets progressively harder when I try it in a shader. :)

	I've run into that classic up-vector, camera flipping problem... At least, I think that's the problem? 
	Anyway, I'm hoping the solution is simple, and that someone reading this will be able to point me in 
	the right direction.

	For now, I've set up a set of 16 random looping points that the camera seems reasonably comfortable 
	with. Just for the record, the general setup works nicely, until the camera loops back on itself in 
	the YZ plane. I'm guessing that increasing the number of way points may eradicate some of the
    intermittent camera spinning, but I figured I'd leave things alone and treat it as a feature. :)

	By the way, I was thankful to have Otavio Good's spline setup in his "Alien Beacon" shader as a 
	reference. On a side note, that particular shader is one of my all time favorites on this site.

	The rendering materials are slightly inspired by the Steampunk genre. Timber, granite, brass, etc. 
	It needs spinning turbines, gears, rivots, and so forth, but that stuff's expensive. Maybe later. 
	Tambako	Jaguar did a really cool shader in the Steampunk aesthetic. The link is below.

	Besides camera path, there's a whole bunch of improvements I'd like to make to this. I've relied on
	occlusion to mask the fact that there are no shadows. I'm hoping to free up some cycles, so I can put
	them back in. I'd also like to add extra detail, but that also slows things down. As for the comments,
	they're very rushed, but I'll tidy those up as well.

	References:

	Alien Beacon - Otavio Good
	https://www.shadertoy.com/view/ld2SzK

    Steampunk Turbine - TambakoJaguar
    https://www.shadertoy.com/view/lsd3zf
    
    // The main inspiration for this shader.
	Mandelmaze in Daylight - dr2
    https://www.shadertoy.com/view/MdVGRc

*/

const float FAR = 50.0; // Far plane.

// Used to identify individual scene objects. In this case, there are only three: The metal framework, the gold
// and the timber.
float objID = 0.; // Wood = 1., Metal = 2., Gold = 3..

// Simple hash function.
float hash( float n ){ return fract(cos(n)*45758.5453); }



// Tri-Planar blending function. Based on an old Nvidia writeup:
// GPU Gems 3 - Ryan Geiss: https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch01.html
vec3 tex3D(sampler2D t, in vec3 p, in vec3 n ){
    
    n = max(abs(n), 0.001);
    n /= dot(n, vec3(1));
	vec3 tx = texture(t, p.yz).xyz;
    vec3 ty = texture(t, p.zx).xyz;
    vec3 tz = texture(t, p.xy).xyz;
    
    // Textures are stored in sRGB (I think), so you have to convert them to linear space 
    // (squaring is a rough approximation) prior to working with them... or something like that. :)
    // Once the final color value is gamma corrected, you should see correct looking colors.
    return (tx*tx*n.x + ty*ty*n.y + tz*tz*n.z);
    
}

// Common formula for rounded squares, for all intended purposes.
float lengthN(in vec2 p, in float n){ p = pow(abs(p), vec2(n)); return pow(p.x + p.y, 1.0/n); }


// The camera path: There are a few spline setups on Shadertoy, but this one is a slight variation of
// Otavio Good's spline setup in his "Alien Beacon" shader: https://www.shadertoy.com/view/ld2SzK
//
// Spline point markers ("cp" for camera point). The camera visits each point in succession, then loops
// back to the first point, when complete, in order to repeat the process. In case it isn't obvious, each 
// point represents an open space juncture in the object that links to the previous and next point.
// Of course, running a camera in a straight line between points wouldn't produce a smooth camera effect, 
// so we apply the Catmull-Rom equation to the line segment.
vec3 cp[16];

void setCamPath(){
    
    // The larger fractal object has nodes in a 4x4x4 grid.
    // The smaller one in a 2x2x2 grid. The following points
    // map a path to various open areas throughout the object.
    const float sl = 2.*.96;
    const float bl = 4.*.96;
    
    cp[0] = vec3(0, 0, 0);
    cp[1] = vec3(0, 0, bl);
    cp[2] = vec3(sl, 0, bl);
    cp[3] = vec3(sl, 0, sl);
    cp[4] = vec3(sl, sl, sl);	
    cp[5] = vec3(-sl, sl, sl);	
    cp[6] = vec3(-sl, 0, sl);
    cp[7] = vec3(-sl, 0, 0);
    
    cp[8] = vec3(0, 0, 0);	
    cp[9] = vec3(0, 0, -bl);
    cp[10] = vec3(0, bl, -bl);	
    cp[11] = vec3(-sl, bl, -bl);
    cp[12] = vec3(-sl, 0, -bl);
    cp[13] = vec3(-sl, 0, 0);
    cp[14] = vec3(-sl, -sl, 0);
    cp[15] = vec3(0, -sl, 0); 
    
    // Tighening the radius a little, so that the camera doesn't hit the walls.
    // I should probably hardcode this into the above... Done.
    //for(int i=0; i<16; i++) cp[i] *= .96;
    
}

// Standard Catmull-Rom equation. The equation takes in the line segment end points (p1 and p2), the
// points on either side (p0 and p3), the current fractional distance (t) along the segment, then
// returns the the smooth (cubic interpolated) position. The end result is a smooth transition 
// between points... Look up a diagram on the internet. That should make it clearer.
vec3 Catmull(vec3 p0, vec3 p1, vec3 p2, vec3 p3, float t){

    return (((-p0 + p1*3. - p2*3. + p3)*t*t*t + (p0*2. - p1*5. + p2*4. - p3)*t*t + (-p0 + p2)*t + p1*2.)*.5);
   
}
 
// Camera path. Determine the segment number (segNum), and how far - timewise - we are along it (segTime).
// Feed the segment, the appropriate adjoining segments, and the segment time into the Catmull-Rom
// equation to produce a camera position. The process is pretty simple, once you get the hang of it.
vec3 camPath(float t){
    
    const int aNum = 16;
    
    t = fract(t/float(aNum))*float(aNum);	// Repeat every 16 time units.
    
    // Segment number. Range: [0, 15], in this case.
    float segNum = floor(t);
    // Segment portion. Analogous to how far we are alone the individual line segment. Range: [0, 1].
    float segTime = t - segNum; 
    
    
    if (segNum == 0.) return Catmull(cp[aNum-1], cp[0], cp[1], cp[2], segTime); 
    
    for(int i=1; i<aNum-2; i++){
        if (segNum == float(i)) return Catmull(cp[i-1], cp[i], cp[i+1], cp[i+2], segTime); 
    }
    
    if (segNum == float(aNum-2)) return Catmull(cp[aNum-3], cp[aNum-2], cp[aNum-1], cp[0], segTime); 
    if (segNum == float(aNum-1)) return Catmull(cp[aNum-2], cp[aNum-1], cp[0], cp[1], segTime);

    return vec3(0);
    
}

// Smooth minimum function. There are countless articles, but IQ explains it best here:
// https://iquilezles.org/articles/smin
float sminP( float a, float b, float s ){

    float h = clamp( 0.5+0.5*(b-a)/s, 0.0, 1.0 );
    return mix( b, a, h ) - s*h*(1.0-h);
}

// Creating the scene geometry.
//
// There are two intertwined fractal objects. One is a gold and timber lattice, spread out in a 4x4x4
// grid. The second is some metallic tubing spread out over a 2x2x2 grid. Each are created by combining
// repeat objects with various operations. All of it is pretty standard.
//
// The code is a little fused together, in order to save some cycles, but if you're interested in the 
// process, I have a "Menger Tunnel" example that's a little easier to decipher.
float map(in vec3 q){

    
///////////

    // The grey section. I have another Menger example, if you'd like to look into that more closely.
    // Layer one.
 	vec3 p = abs(fract(q/4.)*4. - 2.);
 	float tube = min(max(p.x, p.y), min(max(p.y, p.z), max(p.x, p.z))) - 4./3. - .015;// + .05;
    

    // Layer two.
    p = abs(fract(q/2.)*2. - 1.);
 	//d = max(d, min(max(p.x, p.y), min(max(p.y, p.z), max(p.x, p.z))) - s/3.);// + .025
 	tube = max(tube, sminP(max(p.x, p.y), sminP(max(p.y, p.z), max(p.x, p.z), .05), .05) - 2./3.);// + .025
   
///////
    // The gold and timber paneling.
    //
    // A bit of paneling, using a combination of repeat objects. We're doing it here in layer two, just
    // to save an extra "fract" call. Very messy, but saves a few cycles... maybe.
    
    //float panel = sminP(length(p.xy),sminP(length(p.yz),length(p.xz), 0.25), 0.125)-0.45; // EQN 1
    //float panel = sqrt(min(dot(p.xy, p.xy),min(dot(p.yz, p.yz),dot(p.xz, p.xz))))-0.5; // EQN 2
    //float panel = min(max(p.x, p.y),min(max(p.y, p.z),max(p.x, p.z)))-0.5; // EQN 3
    float panel = sminP(max(p.x, p.y),sminP(max(p.y, p.z),max(p.x, p.z), .125), .125)-0.5; // EQN 3
    

    // Gold strip. Probably not the best way to do this, but it gets the job done.
    // Identifying the gold strip region, then edging it out a little... for whatever reason. :)
    float strip = step(p.x, .75)*step(p.y, .75)*step(p.z, .75);
    panel -= (strip)*.025;     
    
    // Timber bulge. Just another weird variation.
    //float bulge = (max(max(p.x, p.y), p.z) - .55);//length(p)-1.;//
    //panel -= bulge*(1.-step(p.x, .75)*step(p.y, .75)*step(p.z, .75))*bulge*.25;    
    
    // Repeat field entity two, which is just an abstract object repeated every half unit. 
    p = abs(fract(q*2.)*.5 - .25);
    float pan2 = min(p.x, min(p.y,p.z))-.05;    
    
    // Combining the two entities above.
    panel = max(abs(panel), abs(pan2)) - .0425;    
/////////
    
    // Layer three. 3D space is divided by three.
    p = abs(fract(q*1.5)/1.5 - 1./3.);
 	tube = max(tube, min(max(p.x, p.y), min(max(p.y, p.z), max(p.x, p.z))) - 2./9. + .025); // + .025 


    // Layer three. 3D space is divided by two, instead of three, to give some variance.
    p = abs(fract(q*3.)/3. - 1./6.);
 	tube = max(tube, min(max(p.x, p.y), min(max(p.y, p.z), max(p.x, p.z))) - 1./9. - .035); //- .025 
    

    
    
    // Object ID: Equivalent to: if(tube<panel)objID=2; else objID = 1.; //etc.
    //
    // By the way, if you need to identify multiple objects, you're better off doing it in a seperate pass, 
    // after the raymarching function. Having multiple "if" statements in a distance field equation can 
    // slow things down considerably.
        
    //objID = 2. - step(tube, panel) + step(panel, tube)*(strip);
    objID = 1.+ step(tube, panel) + step(panel, tube)*(strip)*2.;
    //objID = 1. + step(panel, tube)*(strip) + step(tube, panel)*2.;
    

    return min(panel, tube);
    
    
}

float trace(in vec3 ro, in vec3 rd){

    float t = 0., h;
    for(int i = 0; i < 92; i++){
    
        h = map(ro+rd*t);
        // Note the "t*b + a" addition. Basically, we're putting less emphasis on accuracy, as
        // "t" increases. It's a cheap trick that works in most situations... Not all, though.
        if(abs(h)<.001*(t*.25 + 1.) || t>FAR) break; // Alternative: 0.001*max(t*.25, 1.)
        t += h*.8;
        
    }

    return t;
}


// The reflections are pretty subtle, so not much effort is being put into them. Only 16 iterations.
float refTrace(vec3 ro, vec3 rd){

    float t = 0.;
    for(int i=0; i<16; i++){
        float d = map(ro + rd*t);
        if (d < .0025*(t*.25 + 1.) || t>FAR) break;
        t += d;
    } 
    return t;
}



/*
// Tetrahedral normal, to save a couple of "map" calls. Courtesy of IQ.
vec3 calcNormal(in vec3 p){

    // Note the slightly increased sampling distance, to alleviate artifacts due to hit point inaccuracies.
    vec2 e = vec2(0.0025, -0.0025); 
    return normalize(e.xyy * map(p + e.xyy) + e.yyx * map(p + e.yyx) + e.yxy * map(p + e.yxy) + e.xxx * map(p + e.xxx));
}
*/

// Standard normal function. It's not as fast as the tetrahedral calculation, but more symmetrical. Due to 
// the intricacies of this particular scene, it's kind of needed to reduce jagged effects.
vec3 calcNormal(in vec3 p) {
	const vec2 e = vec2(0.005, 0);
	return normalize(vec3(map(p + e.xyy) - map(p - e.xyy), map(p + e.yxy) - map(p - e.yxy),	map(p + e.yyx) - map(p - e.yyx)));
}

// I keep a collection of occlusion routines... OK, that sounded really nerdy. :)
// Anyway, I like this one. I'm assuming it's based on IQ's original.
float calcAO(in vec3 pos, in vec3 nor)
{
	float sca = 2.0, occ = 0.0;
    for( int i=0; i<5; i++ ){
    
        float hr = 0.01 + float(i)*0.5/4.0;        
        float dd = map(nor * hr + pos);
        occ += (hr - dd)*sca;
        sca *= 0.7;
    }
    return clamp( 1.0 - occ, 0.0, 1.0 );    
}


// Texture bump mapping. Four tri-planar lookups, or 12 texture lookups in total. I tried to 
// make it as concise as possible. Whether that translates to speed, or not, I couldn't say.
vec3 texBump( sampler2D tx, in vec3 p, in vec3 n, float bf){
   
    const vec2 e = vec2(0.001, 0);
    
    // Three gradient vectors rolled into a matrix, constructed with offset greyscale texture values.    
    mat3 m = mat3( tex3D(tx, p - e.xyy, n), tex3D(tx, p - e.yxy, n), tex3D(tx, p - e.yyx, n));
    
    vec3 g = vec3(0.299, 0.587, 0.114)*m; // Converting to greyscale.
    g = (g - dot(tex3D(tx,  p , n), vec3(0.299, 0.587, 0.114)) )/e.x; g -= n*dot(n, g);
                      
    return normalize( n + g*bf ); // Bumped normal. "bf" - bump factor.
	
}


void mainImage( out vec4 fragColor, in vec2 fragCoord ){
   
    
	// Screen coordinates.
	vec2 u = (fragCoord - iResolution.xy*0.5)/iResolution.y;
    
    float speed = iTime*0.35 + 8.;
    
    // Initiate the camera path spline points. Kind of wasteful not making this global, but I wanted
    // it self contained... for better or worse. I'm not really sure what the GPU would prefer.
    setCamPath();

    
	// Camera Setup.
    vec3 ro = camPath(speed); // Camera position, doubling as the ray origin.
    vec3 lk = camPath(speed + .5);  // "Look At" position.
    vec3 lp = camPath(speed + .5) + vec3(0, .25, 0); // Light position, somewhere near the moving camera.
   
    
    // Using the above to produce the unit ray-direction vector.
    float FOV = 1.57; // FOV - Field of view.
    vec3 fwd = normalize(lk-ro);
    vec3 rgt = normalize(vec3(fwd.z, 0, -fwd.x));
    vec3 up = (cross(fwd, rgt));
    
        // Unit direction ray.
    vec3 rd = normalize(fwd + FOV*(u.x*rgt + u.y*up));
    
    
    // Raymarch the scene.
    float t = trace(ro, rd);
    
    // Initialize the scene color.
    vec3 col = vec3(0);
    
    // Scene hit, so color the pixel. Technically, the object should always be hit, so it's tempting to
    // remove this entire branch... but I'll leave it, for now.
    if(t<FAR){
        
        // This looks a little messy and haphazard, but it's really just some basic lighting, and application
        // of the following material properties: Wood = 1., Metal = 2., Gold = 3..
    
        float ts = 1.;  // Texture scale.
        
        // Global object ID. It needs to be saved just after the raymarching equation, since other "map" calls,
        // like normal calculations will give incorrect results. Found that out the hard way. :)
        float saveObjID = objID; 
        
        
        vec3 pos = ro + rd*t; // Scene postion.
        vec3 nor = calcNormal(pos); // Normal.
        vec3 sNor = nor;
        
        
        // Apply some subtle texture bump mapping to the panels and the metal tubing.
        //nor = texBump(iChannel0, pos*ts, nor, 0.002); // + step(saveObjID, 1.5)*0.002
    
        // Reflected ray. Note that the normal is only half bumped. It's fake, but it helps
        // taking some of the warping effect off of the reflections.
        vec3 ref = reflect(rd, normalize(sNor*.5 + nor*.5)); 
         
        
		//col = tex3D(iChannel0, pos*ts, nor); // Texture pixel at the scene postion.
        
        
        vec3  li = lp - pos; // Point light.
        float lDist = max(length(li), .001); // Surface to light distance.
        float atten = 1./(1.0 + lDist*0.125 + lDist*lDist*.05); // Light attenuation.
        li /= lDist; // Normalizing the point light vector.
        
        float occ = calcAO( pos, nor ); // Occlusion.
		
        float dif = clamp(dot(nor, li), 0.0, 1.0); // Diffuse.
        dif = pow(dif, 4.)*2.;
        float spe = pow(max(dot(reflect(-li, nor), -rd), 0.), 8.); // Object specular.
        float spe2 = spe*spe; // Global specular.
        
        float refl = .35; // Reflection coefficient. Different for different materials.

            

        // Reflection color. Mostly fake.
        // Cheap reflection: Not entirely accurate, but the reflections are pretty subtle, so not much 
        // effort is being put in.
        float rt = refTrace(pos + ref*0.1, ref); // Raymarch from "sp" in the reflected direction.
        float rSaveObjID = objID; // IDs change with reflection. Learned that the hard way. :)
        vec3 rsp = pos + ref*rt; // Reflected surface hit point.
        vec3 rsn = calcNormal(rsp); // Normal at the reflected surface. Too costly to bump reflections.
        vec3 rCol = vec3(0); //tex3D(iChannel0, rsp*ts, rsn); // Texel at "rsp."
        vec3 rLi = lp-rsp;
        float rlDist = max(length(rLi), 0.001);
        rLi /= rlDist;
        float rDiff = max(dot(rsn, rLi), 0.); // Diffuse light at "rsp."
        rDiff = pow(rDiff, 4.)*2.;
        float rAtten = 1./(1. + rlDist*0.125 + rlDist*rlDist*.05);
        
        if(rSaveObjID>1.5 && rSaveObjID<2.5){
            rCol = vec3(1)*dot(rCol, vec3(.299, .587, .114))*.7 + rCol*.15;//*.7+.2
            //rDiff *= 1.35;
        }
        if(rSaveObjID>2.5){
             //float rc = dot(rCol, vec3(.299, .587, .114));
             vec3 rFire = pow(vec3(1.5, 1, 1)*rCol, vec3(8, 2, 1.5));//*.5+rc*.5;
             rCol = min(mix(vec3(1.5, .9, .375), vec3(.75, .375, .3), rFire), 2.)*.5 + rCol;         
        }
        
        rCol *= (rDiff + .35)*rAtten; // Reflected color. Not accurate, but close enough.         
        
        
        
        // Grey metal inner tubing.
        if(saveObjID>1.5 && saveObjID<2.5){ 
			
            // Grey out the limestone wall color.
            col = vec3(1)*dot(col, vec3(.299, .587, .114))*.7 + col*.15;
            
            refl = .5;
            //dif *= 1.35;
            //spe2 *= 1.35;
            
        }         
        
        // Gold trimming properties. More effort should probably be put in here.
        // I could just write "saveObjID == 3.," but I get a little paranoid where floats are concerned. :)
        if(saveObjID>2.5){

            // For the screen image, we're interested in the offset height and depth positions. Ie: pOffs.zy.
            
            // Pixelized dot pattern shade.
            //float c = dot(col, vec3(.299, .587, .114));
            
            vec3 fire = pow(vec3(1.5, 1, 1)*col, vec3(8, 2, 1.5));//*.5+c*.5;
            col = min(mix(vec3(1, .9, .375), vec3(.75, .375, .3), fire), 2.)*.5 + col;//
            
            refl = .65;
            //dif *= 1.5;
            //spe2 *= 1.5;
            
        }
        
     
        // Combining everything together to produce the scene color.
        col = col*(dif + .35  + vec3(.35, .45, .5)*spe) + vec3(.7, .9, 1)*spe2 + rCol*refl;
        col *= occ*atten; // Applying occlusion.
       
        
    }

    
    // Applying some very slight fog in the distance. This is technically an inside scene...
    // Or is it underground... Who cares, it's just a shader. :)
    col = mix(min(col, 1.), vec3(0), 1.-exp(-t*t/FAR/FAR*20.));//smoothstep(0., FAR-20., t)
    //col = mix(min(col, 1.), vec3(0), smoothstep(0., FAR-35., t));//smoothstep(0., FAR-20., t)
    
    
    
    // Done.
    fragColor = vec4(sqrt(max(col, 0.)), 1.0);
    
}
 
void main() {
    vec4 color;
	mainImage(color, texCoord);
    fragColor = color;
})";

class TMyApp
{
	private:
		static const glm::vec2 screen;
		static const char caption[];
		static float quadVerts[];
		bool is_parent_console;
		uint32_t con_cp;
		bool is_fullscreen;
		bool is_screensaver;
		int i_mon_cnt;
		int i_wnd_cnt;
		GLFWmonitor** mon;
		int wnd_pos[2], wnd_size[2];
		
		//GLuint framebuffer;
		unsigned int shaderProgram;
		GLuint VAO;
		GLFWwindow** wnd;
		float *pf_time;
		float lastTime;
		
		string ConvertUTF8ToCp(const string& str);
		bool is_any_wnd_should_close();
		void init_wnd(GLFWwindow *wnd, int width, int height);
		void set_mode(void);
		void on_size(GLFWwindow* wnd, int width, int height);
		void on_key(GLFWwindow* wnd, int key, int scancode, int action, int mods);
		void on_mouse_pos(GLFWwindow* wnd, double xpos, double ypos);
		void on_mouse_btn(GLFWwindow* wnd, int button, int action, int mods);
		void draw(void);
		inline bool is_preview(void) const;
		void init(bool is_screensaver, bool is_fullscreen, bool is_visible);
		void show_usage(void);

	public:
		//TMyApp(bool is_visible = true);
		TMyApp(int argc, char *argv[]);
		~TMyApp();
		
		void run(void);
}; // class TMyApp

const glm::vec2 TMyApp::screen(1, 1);

const char TMyApp::caption[] = "alphawater";

float TMyApp::quadVerts[] = {
	-1.0, -1.0,     0.0, 0.0,
	-1.0, 1.0,      0.0, 1.0,
	1.0, -1.0,      1.0, 0.0,

	1.0, -1.0,      1.0, 0.0,
	-1.0, 1.0,      0.0, 1.0,
	1.0, 1.0,       1.0, 1.0
};

string TMyApp::ConvertUTF8ToCp(const string& str)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    wchar_t* wstr = new wchar_t[len];
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wstr, len);

    len = WideCharToMultiByte(con_cp, 0, wstr, -1, NULL, 0, NULL, NULL);
    char* s = new char[len];
    WideCharToMultiByte(con_cp, 0, wstr, -1, s, len, NULL, NULL);

    std::string result(s);
    delete[] wstr;
    delete[] s;

    return result;
}

void TMyApp::init_wnd(GLFWwindow *wnd, int width, int height)
{
	if(is_fullscreen)
	{
		on_size(wnd, width, height);
	}

	glfwSetWindowUserPointer(wnd, this);
	{
		auto cb = [](GLFWwindow* wnd, int width, int height)
		{
			TMyApp *o = reinterpret_cast<TMyApp *>(glfwGetWindowUserPointer(wnd));
			o->on_size(wnd, width, height);
		};
		glfwSetFramebufferSizeCallback(wnd, cb);
	}
	{
		auto cb = [](GLFWwindow* wnd, int key, int scancode, int action, int mods)
		{
			TMyApp *o = reinterpret_cast<TMyApp *>(glfwGetWindowUserPointer(wnd));
			o->on_key(wnd, key, scancode, action, mods);
		};	
		glfwSetKeyCallback(wnd, cb);
	}

	if(is_screensaver)
	{
		{
			auto cb = [](GLFWwindow* wnd, double xpos, double ypos)
			{
				TMyApp *o = reinterpret_cast<TMyApp *>(glfwGetWindowUserPointer(wnd));
				o->on_mouse_pos(wnd, xpos, ypos);
			};
			glfwSetCursorPosCallback(wnd, cb);
		}
		{
			auto cb = [](GLFWwindow* wnd, int button, int action, int mods)
			{
				TMyApp *o = reinterpret_cast<TMyApp *>(glfwGetWindowUserPointer(wnd));
				o->on_mouse_btn(wnd, button, action, mods);
			};
			glfwSetMouseButtonCallback(wnd, cb);
		}
	}

	glfwMakeContextCurrent(wnd);
	
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cerr << "failed to initialize glad with processes" << endl;
		exit(-1);
	}

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(0));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	GLuint framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer); 

	GLuint texColor;
	glGenTextures(1, &texColor);
	glBindTexture(GL_TEXTURE_2D, texColor);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColor, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	////////
	//vertex shader
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	// fragment shader
	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	// check for shader compile errors

	// link shaders
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	// check for linking errors

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glUseProgram(shaderProgram);
	glUniform2fv(glGetUniformLocation(shaderProgram, "iResolution"), 1, &screen[0]);
}

void TMyApp::set_mode(void)
{
    if (is_fullscreen)
    {
        // backup wnd position and wnd size
        glfwGetWindowPos(wnd[0], &wnd_pos[0], &wnd_pos[1] );
        glfwGetWindowSize(wnd[0], &wnd_size[0], &wnd_size[1] );

        // get resolution of monitor
        const GLFWvidmode * mode = glfwGetVideoMode(mon[0]);
        // switch to full screen
        glfwSetWindowMonitor(wnd[0], mon[0], 0, 0, mode->width, mode->height, 0);
		glfwSetInputMode(wnd[0], GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

		i_wnd_cnt = i_mon_cnt;
		for(int i = 1; i < i_wnd_cnt; i++)
		{
			int width;
			int height;

			mode = glfwGetVideoMode(mon[i]);
			width = mode->width;
			height = mode->height;

			wnd[i] = glfwCreateWindow(width, height, caption, mon[i], nullptr);
			if (!wnd[i])
			{
				cerr << "failed to create wnd" << endl;
				exit(-1);
			}

			glfwSetInputMode(wnd[i], GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
			init_wnd(wnd[i], width, height);
		}
    }
    else
    {
		for(int i = 1; i < i_wnd_cnt; i++)
		{
			glfwDestroyWindow(wnd[i]);
			wnd[i] = NULL;
		}

		i_wnd_cnt = 1;
        // restore last wnd size and position
        glfwSetWindowMonitor(wnd[0], nullptr,  wnd_pos[0], wnd_pos[1], wnd_size[0], wnd_size[1], 0);

		glfwSetInputMode(wnd[0], GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

void TMyApp::on_size(__attribute__((unused)) GLFWwindow* wnd, int width, int height)
{
	cout << "wnd=" << wnd << "; w=" << width << "; h=" << height << endl;
	glfwMakeContextCurrent(wnd);
	int t = max(width, height);
	glViewport(-(t-width)*0.5, -(t-height)*0.5, t, t);
	//glViewport(0, 0, t, t);
    //glViewport(0, 0, width, height);
	
	if(!is_fullscreen)
	{
		draw();
	}
}

void TMyApp::on_key(GLFWwindow* wnd, int key, __attribute__((unused)) int scancode, int action, int mods)
{
	static bool is_mode_switch = false;

	if(is_screensaver)
	{
		if(!is_preview())
		{
			glfwSetWindowShouldClose(wnd, true);
		}
	}
	else
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(wnd, true);
		}
		else
		if (key == GLFW_KEY_ENTER && action == GLFW_PRESS && (mods & GLFW_MOD_ALT))
		{
			if(!is_mode_switch)
			{
				is_mode_switch = true;
				is_fullscreen = !is_fullscreen;
				set_mode();
			}
		}
		else
		{
			is_mode_switch = false;
		}
	}
}

void TMyApp::on_mouse_pos(GLFWwindow* wnd, __attribute__((unused)) double xpos, __attribute__((unused)) double ypos)
{
	static bool is_first_run = true;
	
	if(is_screensaver && !is_preview())
	{
		if(is_first_run)
		{
			is_first_run = false;
		}
		else
		{
			glfwSetWindowShouldClose(wnd, true);
		}
	}
}

void TMyApp::on_mouse_btn(GLFWwindow* wnd, __attribute__((unused)) int button, __attribute__((unused)) int action, __attribute__((unused)) int mods)
{
	if(is_screensaver && !is_preview())
	{
		glfwSetWindowShouldClose(wnd, true);
	}
}

void TMyApp::draw(void)
{
	float now = glfwGetTime();
	float delta = now - lastTime;

	lastTime = now;
	
	for(int i = 0; i < i_wnd_cnt; i++)
	{
		glfwMakeContextCurrent(wnd[i]);
		pf_time[i] += delta;

		glUseProgram(shaderProgram);
		glUniform1f(glGetUniformLocation(shaderProgram, "iTime"), pf_time[i]);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glfwSwapBuffers(wnd[i]);
	}
}

inline bool TMyApp::is_preview(void) const
{
	return is_screensaver && !is_fullscreen;
}

void TMyApp::init(bool is_screensaver, bool is_fullscreen, bool is_visible)
{
    int width;
    int height;
	
	this->is_screensaver = is_screensaver;
	this->is_fullscreen = is_fullscreen;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
	glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_FALSE);

	if(!is_visible)
	{
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	}

	mon = glfwGetMonitors(&i_mon_cnt);
	cout << i_mon_cnt << " monitors found." << endl;
	wnd = new GLFWwindow * [i_mon_cnt];
	pf_time = new float[i_mon_cnt];
	for(int i = 0; i < i_mon_cnt; i++)
	{
		pf_time[i] = rand() % 1000;
	}
	
	if (is_fullscreen)
	{
		i_wnd_cnt = i_mon_cnt;
		for(int i = i_wnd_cnt - 1; i >= 0; i--)
		{
			const GLFWvidmode* mode = glfwGetVideoMode(mon[i]);
			width = mode->width;
			height = mode->height;
			cout << "mon[" << i << "] = " << mon[i] << endl;
			wnd[i] = glfwCreateWindow(width, height, caption, mon[i], nullptr);
			if (!wnd[i])
			{
				cerr << "failed to create wnd" << endl;
				exit(-1);
			}

			glfwSetInputMode(wnd[i], GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		}

		if(i_wnd_cnt > 0)
		{
			int dt = 100;
			wnd_pos[0] = dt;
			wnd_pos[1] = dt;
			dt <<= 1;
			wnd_size[0] = width - dt;
			wnd_size[1] = height - dt;
		}
	}
	else
	{
		i_wnd_cnt = 1;
		width = 200;
		height = 200;
		
		for(int i = i_wnd_cnt - 1; i >= 0; i--)
		{
			string cap = caption;
			cap += " ";
			cap += to_string(i);
			//wnd[i] = glfwCreateWindow(width, height, caption, nullptr, nullptr);
			wnd[i] = glfwCreateWindow(width, height, cap.c_str(), nullptr, nullptr);
			if (!wnd[i])
			{
				cerr << "failed to create wnd" << endl;
				exit(-1);
			}
		}

        glfwGetWindowSize(wnd[0], &wnd_size[0], &wnd_size[1]);
        glfwGetWindowPos(wnd[0], &wnd_pos[0], &wnd_pos[1]);
	}

	for(int i = i_wnd_cnt - 1; i >= 0; i--)
	{
		//MessageBoxA(NULL, "debug", "HIT.1", MB_OK);
		init_wnd(wnd[i], width, height);

		lastTime = glfwGetTime();
		
		//glfwMaximizeWindow(wnd[i]);
	}
}

void TMyApp::show_usage(void)
{
	string s = "Неверные параметры!\n"
			"Использование:\n"
			"alphawater.exe [/c] | [/p <HWND>] | [/s]\n"
			"\t/c             - показать диалог настроек\n"
			"\t/p <HWND>      - предпросмотр в окне, дочернем по отношению к <HWND>\n"
			"\t/s             - запуск в полноэкранном режиме\n\n"
			"\tБез параметров - запуск в оконном режиме.";
			
	cout << ConvertUTF8ToCp(s) << endl;
}

TMyApp::TMyApp(int argc, char *argv[])
{
	is_parent_console = AttachConsole(ATTACH_PARENT_PROCESS);

	if(is_parent_console)
	{
		freopen("CON", "w", stdout);
		freopen("CON", "r", stdin);
		freopen("CON", "w", stderr);
		cout << endl;
		
		con_cp = GetConsoleOutputCP();
	}

	switch(argc)
	{
		case 1 + 2:
		{
			if(!strcmp(argv[1], "/p"))
			{
				init(true, false, false);

				char *s_ptr = argv[2];
				HWND h_wnd_parent = (HWND)stoull(s_ptr, nullptr, 10);
				HWND h_wnd = glfwGetWin32Window(wnd[0]);
				SetParent(h_wnd, h_wnd_parent);
				SetWindowLong(h_wnd, GWL_STYLE, WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN);
				RECT rc;
				GetClientRect(h_wnd_parent, &rc);
				MoveWindow(h_wnd, rc.left, rc.top, rc.right, rc.bottom, TRUE);
			}
			else
			{
				show_usage();
				throw exception();
			}
			break;
		}
		
		case 1 + 1:
		{
			if(!strcmp(argv[1], "/s"))
			{
				init(true, true, true);
			}
			else
			if(!strcmp(argv[1], "/c"))
			{
			}
			else
			{
				show_usage();
				throw exception();
			}
			break;
		}
		
		default:
		{
			init(false, false, true);
		}
	}
}

TMyApp::~TMyApp()
{
	cout << "exit" << endl;
	
	if(is_parent_console)
	{
		//MessageBoxA(NULL, to_string(FreeConsole()).c_str(), "debug", MB_OK);
		FreeConsole();
	}

    glfwTerminate();
    // cleanup
}

bool TMyApp::is_any_wnd_should_close()
{
	bool res = false;

	for(int i = i_wnd_cnt - 1; i >= 0; i--)
	{
		if(glfwWindowShouldClose(wnd[i]))
		{
			res = true;
			break;
		}
	}
	return res;
}

void TMyApp::run(void)
{
    while (!is_any_wnd_should_close())
    {
		draw();
        glfwPollEvents();
    }
}

int main(int argc, char *argv[])
{
	int res;

	try
	{
		TMyApp app(argc, argv);
		app.run();
		res = 0;
	}
	catch(exception& )
	{
		res = 1;
	}
	
	return res;
}