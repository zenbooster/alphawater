vec2 loadVec2(ivec2 ID) { return texelFetch(iChannel0, ID, 0).xy;}
vec4 loadVec4(ivec2 ID) { return texelFetch(iChannel0, ID, 0);}
void saveVec2(ivec2 ID, vec2 value, inout vec4 fragColor, ivec2 fragCoordInt) { if (fragCoordInt == ID) fragColor = vec4(value, 0.0, 1.0); }
void saveVec4(ivec2 ID, vec4 value, inout vec4 fragColor, ivec2 fragCoordInt) { if (fragCoordInt == ID) fragColor = value; }

void gameLogic(out vec4 fragColor, ivec2 fragCoordI)
{
    fragColor = vec4(0.0);
    saveVec2(IVEC2_RESOLUTION, iResolution.xy, fragColor, fragCoordI);

    //Mouse
    vec4 mouseState = loadVec4(VEC4_MOUSESTATE);
    vec2 mousen= (iMouse.xy - iResolution.xy * 0.5) / iResolution.y * 2.0;
    vec2 deltaMouse=mousen-mouseState.xy;
    if (iMouse.w>0.)  deltaMouse=vec2(.0);
    mouseState.xy = mousen;
    if (iMouse.z > 0.) mouseState.z=0.;
    mouseState.z+=iTimeDelta;   //Time since last interaction
    saveVec4(VEC4_MOUSESTATE, mouseState, fragColor, fragCoordI);
    bool isInsideGame=max(abs(mouseState.x), abs(mouseState.y)) < 1.;
    
    //Petting
    vec4 petting = loadVec4(VEC4_PETTING);
    petting.xy = mouseState.xy;
    if (iMouse.z > 0.0 && isInsideGame ) petting.z = clamp(petting.z+length(deltaMouse)*4.,0.,1.);
    petting.z = clamp(petting.z - iTimeDelta*.5, 0.0, 1.0);

    //Gain love
    petting.w += petting.z*iTimeDelta*0.1;
    petting.w = clamp(petting.w - iTimeDelta*.05, 0.0, 1.0);
    saveVec4(VEC4_PETTING, petting, fragColor, fragCoordI);

    //Pupils
    vec4 pupils=loadVec4(VEC4_PUPILS);
    vec4 lookDir=loadVec4(VEC4_LOOK_DIR);
    if (pupils.z>0.)
    {
        pupils.z-=iTimeDelta;
        lookDir.xy=mix(lookDir.xy,pupils.xy,iTimeDelta*5.)*(1.-petting.w);
        lookDir.zw=lookDir.xy;
        if (pupils.z<0.) pupils.w=rand(iTime)*4.+2.;
    }
    else
    {
        pupils.w-=iTimeDelta;
        if (pupils.w<0.)
        {
            pupils.xy=vec2(sin(iTime)*0.3, cos(iTime)*0.1);
            pupils.z=1.;
        }
    }
    saveVec4(VEC4_PUPILS, pupils, fragColor, fragCoordI);
    saveVec4(VEC4_LOOK_DIR, lookDir, fragColor, fragCoordI);
    
    //Eye open
    vec4 blink=loadVec4(VEC4_BLINK);
    vec4 eyeOpen=loadVec4(VEC4_EYEOPEN);
    if (blink.z>0.)
    {
        blink.z-=iTimeDelta*(2.-(petting.w*2.));
        eyeOpen.z=pingpong(1.-blink.z)*blink.x;
        eyeOpen.w=eyeOpen.z;
        if (blink.z<0.) blink.w=rand(iTime)*10.+2.;
    }
    else
    {
        blink.w-=iTimeDelta;
        if (blink.w<0.&&eyeOpen.x>0.3)
        {
            blink.x=eyeOpen.x;
            blink.z=1.;
        }
    }
    float eyeSpeed=iTimeDelta*3.;
    eyeOpen.x=moveTowards(eyeOpen.x,eyeOpen.z,eyeSpeed);
    eyeOpen.y=moveTowards(eyeOpen.y,eyeOpen.w,eyeSpeed*0.8);

    //Make the eyes close if in love
    eyeOpen.z=1.-petting.w;
    eyeOpen.w=1.-petting.w;

    //Petting the eye?
    if (iMouse.z>0.)
    {
        eyeOpen.z = min(eyeOpen.z, step(0.3, distance(mouseState.xy, eye0)));
        eyeOpen.w = min(eyeOpen.w, step(0.3, distance(mouseState.xy, eye1)));
    }
    saveVec4(VEC4_EYEOPEN,eyeOpen,fragColor,fragCoordI);
    saveVec4(VEC4_BLINK,blink,fragColor,fragCoordI);

    //Face dir
    vec4 faceDir=loadVec4(VEC4_FACE_DIR);
    float faceSpeed=iTimeDelta*0.5;
    faceDir.x=moveTowards(faceDir.x,faceDir.z,faceSpeed);
    faceDir.y=moveTowards(faceDir.y,faceDir.w,faceSpeed);
    if (iMouse.z>0. && isInsideGame)
        faceDir.zw=mouseState.xy*2.;
    else
        faceDir.zw=vec2(sin(iTime*0.4876),cos(iTime*0.437)*0.3);
    saveVec4(VEC4_FACE_DIR,faceDir,fragColor,fragCoordI);
}
void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    ivec2 fragCoordI = ivec2(fragCoord - 0.5);
    if (fragCoordI.x<10 && fragCoordI.y==0)
        gameLogic(fragColor, fragCoordI);
   else
   {
        if (ivec2(iResolution.xy) != ivec2(loadVec2(IVEC2_RESOLUTION)) || iFrame == 0) 
        {
            //Do the heavy inference here, only update when the resolution changes
            vec2 uv = (fragCoord-iResolution.xy*.5)/iResolution.y*2.;
            float depth=clamp(d(uv),0.,1.);
            float sat=s(uv), gray=v(uv);
            fragColor = vec4(gray,mix(0.1,sat, smoothstep(0.3,1.,1.-depth)),depth,1.);
        } else
            fragColor = texture(iChannel0, fragCoord / iResolution.xy);
   }
}
