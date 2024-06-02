#define T iTime

//IQs noise
float noise(vec3 rp) {
    vec3 ip = floor(rp);
    rp -= ip; 
    vec3 s = vec3(7, 157, 113);
    vec4 h = vec4(0.0, s.yz, s.y + s.z) + dot(ip, s);
    rp = rp * rp * (3.0 - 2.0 * rp); 
    h = mix(fract(sin(h) * 43758.5), fract(sin(h + s.x) * 43758.5), rp.x);
    h.xy = mix(h.xz, h.yw, rp.y);
    return mix(h.x, h.y, rp.z); 
}

float fbm(vec3 x) {
    float r = 0.0;
    float w = 1.0;
    float s = 1.0;
    for (int i = 0; i < 5; i++) {
        w *= 0.5;
        s *= 2.0;
        r += w * noise(s * x);
    }
    return r;
}

// IQ anti-aliasing - see https://www.shadertoy.com/view/MtffWs
vec3 pri(vec3 x) {
    vec3 h = fract(x / 2.0) - 0.5;
    return x * 0.5 + h * (1.0 - 2.0 * abs(h));
}

float checkersTextureGradTri(vec3 p, vec3 ddx, vec3 ddy) {
    vec3 w = max(abs(ddx), abs(ddy)) + 0.01; // filter kernel
    vec3 i = (pri(p + w) - 2.0 * pri(p) + pri(p - w)) / (w * w); // analytical integral (box filter)
    return 0.5 - 0.5 * i.x *  i.y * i.z; // xor pattern
}

vec3 texCoords(vec3 p) {
	return 5.0 * p;
}

//Moody clouds from Patu
//https://www.shadertoy.com/view/4tVXRV
vec3 clouds(vec3 rd) {
    vec2 uv = rd.xz / (rd.y + 0.6);
    float nz = fbm(vec3(uv.yx * 1.4 + vec2(T * 0.013, 0.0), T * 0.013)) * 1.5;
    return clamp(pow(vec3(nz), vec3(4.0)) * rd.y, 0.0, 1.0);
}


vec2 map(vec3 p) {
	p.xz *= rot(T);    
    float ofs = pModPolar(p.xz, 5.0);
    return vec2(dfDog(p - vec3(8.0, 0.0, 0.0), ofs, T), ofs);;    
}

//inner glow
vec2 gmap(vec3 p) {
	p.xz *= rot(T);    
    float ofs = pModPolar(p.xz, 5.0);
    return vec2(gdfDog(p - vec3(8.0, 0.0, 0.0), ofs, T), ofs);;    
}

vec3 normal(vec3 p) {  
    vec2 e = vec2(-1., 1.) * EPS;   
	return normalize(e.yxx * map(p + e.yxx).x + e.xxy * map(p + e.xxy).x + 
					 e.xyx * map(p + e.xyx).x + e.yyy * map(p + e.yyy).x);   
}

float AO(vec3 p, vec3 n) {

    float r = 0.0;
    float w = 1.0;
    float d = 0.0;

    for (float i = 1.0; i < 5.0; i += 1.0){
        d = i / 5.0;
        r += w * (d - map(p + n * d).x);
        w *= 0.5;
    }

    return 1.0 - clamp(r, 0.0, 1.0);
}

//IQ https://www.shadertoy.com/view/lsKcDD
float shadow(vec3 ro, vec3 rd, float tmax) {
	
    float res = 1.0;
    float t = 0.0;
    float ph = 1e10; // big, such that y = 0 on the first iteration
    
    for(int i = 0; i < 32; i++) {
		float h = map(ro + rd * t).x;

        // use this if you are getting artifact on the first iteration, or unroll the
        // first iteration out of the loop
        //float y = (i==0) ? 0.0 : h*h/(2.0*ph); 

        float y = h*h/(2.0*ph);
        float d = sqrt(h*h-y*y);
        res = min( res, 10.0*d/max(0.0,t-y) );
        ph = h;
        
        t += h;
        
        if (res < EPS || t > tmax) break;
    }
    
    return clamp(res, 0.0, 1.0);
}

vec2 march(vec3 ro, vec3 rd) {
    float t = 0.0, id = 0.0;
    vec3 n = vec3(0.);
    for (int i = 0; i < 98; i++) {
        vec3 rp = ro + rd * t;
        vec2 ns = map(rp);
        if (ns.x < EPS || t > FAR) {
            id = ns.y;
            break;
        }
        t += ns.x * 0.6;
    }
    return vec2(t, id);
}

/*
//balloom interior
vec2 vMarch(vec3 ro, vec3 rd) {
    float t = 0.0, gc1 = 0., gc2 = 0.;
    for (int i = 0; i < 96; i++) {
        vec3 rp = ro + rd * t;
        float ns = map(rp).x;
        if (ns > 0.01 || t > FAR) break;
        
        vec2 gns = gmap(rp);
        gc1 += 0.1 / (1.0 + gns.x * gns.x * 10.);
        gc2 += 0.8 / (1.0 + gns.x * gns.x * 100.);
        
        t += 0.1;
    }
    return vec2(gc1, gc2);
}
*/

struct Scene {
    float t;
    float id;
    vec3 n;
    float did;
};

Scene drawScene(vec3 ro, vec3 rd) {

    float mint = FAR, id = 0., did = 0., rf = 0.;
    vec3 minn = vec3(0.);
    
    vec3 fn = vec3(0.0, 1.0, 0.0), fo = vec3(0.0, -2.0, 0.0);
    float ft = planeIntersection(ro, rd, fn, fo);
    if (ft > 0.0 && ft < FAR) {
        mint = ft;
        minn = fn;
        id = 1.0;
    }
    
    vec2 t = march(ro, rd);
        
    if (t.x > 0.0 && t.x < mint) {
        vec3 rp = ro + rd * t.x;
        mint = t.x;
        minn = normal(rp);
        id = 2.0;
        did = t.y;
    }
    
    return Scene(mint, id, minn, did);
}
        
vec3 colourScene(Scene scene, vec3 ro, vec3 rd) {
 
    vec3 pc = vec3(0.);
    vec3 lp = vec3(4.0, 5.0, -2.0);
    
    vec3 rp = ro + rd * scene.t;
    vec3 ld = normalize(lp - rp);
    float lt = length(lp - rp);
    float df = max(dot(ld, scene.n), 0.05);
    float atn = 1.0 / (1.0 + lt * lt * 0.01);
    float sh = 0.5 + 0.5 * shadow(rp, ld, 20.0);
    float sp = pow(max(dot(reflect(-ld, scene.n), -rd), 0.0), 64.0);
    float frs = pow(clamp(dot(scene.n, rd) + 1., 0., 1.), 2.);

    if (scene.id == 1.0) {
        
        // calc texture sampling footprint	
        vec3 uvw = texCoords(rp * 0.08);
		vec3 ddx_uvw = dFdx(uvw); 
    	vec3 ddy_uvw = dFdy(uvw);
        float fc = checkersTextureGradTri(uvw, ddx_uvw, ddy_uvw);
        fc = clamp(fc, 0.2, 0.8);
        pc = ac * (fc + noise(fc + rp * vec3(8.0 * fc, 1.0 *fc * 2.0, 0.5))) * df * atn * sh;

    } else if (scene.id == 2.0) {

        float ao = AO(rp, scene.n);
        df = max(dot(ld, scene.n), 0.8);

        //finally tried implemnting some of IQs suggestions and removed vMarch
        vec3 sc = palette((T + (scene.did + 2.0) * 2.0) * 0.1, CA, CB, CC, CD);
        pc = sc * ao;
        pc *= 1. - ao * 0.6;
        pc += sc * frs * 0.6;
        pc *= df;
        //pc *= df * 2.;
        pc += vec3(1.) * sp;
        
        //vec2 vc = vMarch(rp, rd);;
        //pc += sc * vc.x; //fake balloon volume
        //pc += sc * vc.y * max(sin(T * 0.4), 0.0); //glow
    }
    
    return pc;
}

void setupCamera(vec2 fragCoord, inout vec3 ro, inout vec3 rd) {

    vec2 uv = (fragCoord.xy - iResolution.xy * 0.5) / iResolution.y;

    vec3 lookAt = vec3(0.0, 0.0, 0.0);
    ro = lookAt + vec3(0.0, 6.0 + sin(T * 0.5) * 5.0, -17.0 - sin(T * 0.3) * 3.);

    ro.xz *= rot(T * -0.2);
    
    float FOV = PI / 3.0;
    vec3 forward = normalize(lookAt - ro);
    vec3 right = normalize(vec3(forward.z, 0.0, -forward.x)); 
    vec3 up = cross(forward, right);

    rd = normalize(forward + FOV * uv.x * right + FOV * uv.y * up);
}

vec3 render(vec3 ro, vec3 rd) {
    
    vec3 pc = clouds(rd) * ac * 2.0;
    
    Scene scene = drawScene(ro, rd);
    if (scene.id > 0.0) {
        pc = colourScene(scene, ro, rd);
        //reflection
        vec3 rro = ro + rd * (scene.t - EPS);
        vec3 rrd = reflect(rd, scene.n);
        Scene rs = drawScene(rro, rrd);
        vec3 rc = colourScene(rs, rro, rrd);
        float rfatn = 1.0 / (1.0 + rs.t * rs.t * 0.1);
        pc += rc * rfatn * 0.5;
    }
    
    return pc; 
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {

    vec3 ro, rd;
    setupCamera(fragCoord, ro, rd);
    
    fragColor = vec4(render(ro, rd), 1.0);
}

void mainVR(out vec4 fragColor, in vec2 fragCoord, in vec3 fragRayOri, in vec3 fragRayDir) {    
    fragColor = vec4(render(fragRayOri * 4. + vec3(0.0,1.0,1.5), fragRayDir), 1.0);
}
