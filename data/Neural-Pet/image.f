#define COLOR 0.05
vec4 loadVec4(ivec2 ID) { return texelFetch(iChannel0, ID, 0); }

float drawPupil(vec2 uv, vec2 center, vec2 look, float v, float open) {
    look = look / max(1.0, length(look)); //Workaround for data corruption when shader page loses focus
    vec2 eye = (uv - center) * vec2(1.8, 1.0 / (open * 0.37));
    vec2 pupil = (uv - center + vec2(look.x * 0.07, look.y * 0.04 * open)) * vec2(1.8, 0.9);
    float eyeEdge = 1. - smoothstep(1.0, 1.2, length(eye) / 0.2);
    float pupilEdge = 1. - smoothstep(0.5, 1.0, length(pupil) / 0.06);
    return mix(v, 0., pupilEdge * eyeEdge) + smoothstep(0.4, 0.7, v) * v * eyeEdge * pupilEdge;
}
void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    vec2 uv = (fragCoord - iResolution.xy * 0.5) / iResolution.y * 2.;
    vec2 uvn = fragCoord / iResolution.xy;    
    if (max(abs(uv.x), abs(uv.y)) < 1.) {
    
        vec4 mouseState=loadVec4(VEC4_MOUSESTATE);
    
        vec4 data = texture(iChannel0, uvn);
        float depth = pow(1.0 - data.b, 2.3);
        vec4 faceDir=loadVec4(VEC4_FACE_DIR)*0.1;
        
        vec4 petting=loadVec4(VEC4_PETTING);
        float petDist=clamp(1.-distance(uv,petting.xy)*3.,0.,1.)*petting.z*0.05;
        
        uv += depth * (faceDir.xy+petDist);
        data = texture(iChannel0, vec2(((uv.x / (iResolution.x / iResolution.y)) + 1.) / 2., (uv.y + 1.) / 2.));
        fragColor.rgb = hsv2rgb(COLOR, data.g, data.r);

        vec4 eyeOpen=loadVec4(VEC4_EYEOPEN);
        float currentEyeOpen=eyeOpen.x;
        vec2 euv = (uv + vec2(162., -48.) / 512.) * (512. / 150.);
        if (abs(euv.x) > 1.0 || abs(euv.y) > 1.0)
        {
            currentEyeOpen=eyeOpen.y;
            euv = (vec2(-uv.x, uv.y) + vec2(200., -60.) / 512.) * (512. / 150.);
        }
        #define START 60.
        if (mouseState.z > START) {
            float a=1.-clamp(abs(mod(mouseState.z - START,40.)-3.)/3.,0.,1.);
            float sillyEyes = (sin(uvn.x * 12.0 - iTime * 5.0) + 1.0) * 0.5;
            currentEyeOpen = mix(currentEyeOpen, sillyEyes, a); //ElectroBoom eyewave when bored :D 
        }
        if (abs(euv.x) < 1.0 && abs(euv.y) < 1.0) {
            float frameIndex=clamp(1.-currentEyeOpen,0.,1.)*7.9;
            int frameIndexI = int(frameIndex);
            float eyeV, eyeV0, eyeV1;
            switch (frameIndexI) { 
                case 0: eyeV0 = e0(euv); eyeV1 = e1(euv); break;
                case 1: eyeV0 = e1(euv); eyeV1 = e2(euv); break;
                case 2: eyeV0 = e2(euv); eyeV1 = e3(euv); break;
                case 3: eyeV0 = e3(euv); eyeV1 = e4(euv); break;
                case 4: eyeV0 = e4(euv); eyeV1 = e5(euv); break;
                case 5: eyeV0 = e5(euv); eyeV1 = e6(euv); break;
                case 6: eyeV0 = e6(euv); eyeV1 = e7(euv); break;
                case 7: eyeV0 = e7(euv); eyeV1 = eyeV0; break;
            }
            eyeV=mix(eyeV0,eyeV1,frameIndex-float(frameIndexI));
            vec4 lookDir=loadVec4(VEC4_LOOK_DIR);
            eyeV = drawPupil(uv, eye0, lookDir.xy, eyeV, currentEyeOpen);
            eyeV = drawPupil(uv, eye1, lookDir.zw, eyeV, currentEyeOpen);
            fragColor.rgb = mix(hsv2rgb(COLOR, 0.3, eyeV), fragColor.rgb, smoothstep(0.3, 1.0, clamp(length(euv), 0., 1.)));
        }
    } else
    {
        vec4 c=texture(iChannel0, uvn);
        fragColor.rgb = hsv2rgb(0.05, 0.1, pow((c.r+c.g+c.b)/3.,0.5));
    }
}
