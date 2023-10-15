#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <GL/gl.h>

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
uniform vec2 iMouse;
uniform vec2 iResolution;
uniform float iTime;
out vec4 fragColor;
vec2 fragCoord = gl_FragCoord.xy;
#define PI 3.14159265359
#define TWOPI 6.28318530718
//drag the window LR to control roughness

//--graphics setting (lower = better fps)---------------------------------------------------------------------
#define AVERAGECOUNT 16
#define MAX_BOUNCE 32

//--scene data---------------------------------------------------------------------
#define SPHERECOUNT 6
//xyz = pos, w = radius
const vec4 AllSpheres[SPHERECOUNT]=vec4[SPHERECOUNT](
    vec4(0.0,0.0,0.0,2.0),//sphere A
    vec4(0.0,0.0,-1.0,2.0),//sphere B
    vec4(0.0,-1002.0,0.0,1000.0),//ground
    vec4(0.0,0.0,+1002,1000.0),//back wall
    vec4(-1004.0,0.0,0.0,1000.0),//left wall    
    vec4(+1004.0,0.0,0.0,1000.0)//right wall
);
//-----------------------------------------------------------------------
float raySphereIntersect(vec3 r0, vec3 rd, vec3 s0, float sr) {
    // - r0: ray origin
    // - rd: normalized ray direction
    // - s0: sphere center
    // - sr: sphere radius
    // - Returns distance from r0 to first intersecion with sphere,
    //   or -1.0 if no intersection.
    float a = dot(rd, rd);
    vec3 s0_r0 = r0 - s0;
    float b = 2.0 * dot(rd, s0_r0);
    float c = dot(s0_r0, s0_r0) - (sr * sr);
    if (b*b - 4.0*a*c < 0.0) {
        return -1.0;
    }
    return (-b - sqrt((b*b) - 4.0*a*c))/(2.0*a);
}
//-----------------------------------------------------------------------
struct HitData
{
    float rayLength;
    vec3 normal;
};
HitData AllObjectsRayTest(vec3 rayPos, vec3 rayDir)
{
    HitData hitData;
    hitData.rayLength = 9999.0; //default value if can't hit anything

    for(int i = 0; i < SPHERECOUNT; i++)
    {
        vec3 sphereCenter = AllSpheres[i].xyz;
        float sphereRadius = AllSpheres[i].w;
        //----hardcode sphere pos animations-------------------------------------
        if(i == 0)
        {
            float t = fract(iTime * 0.7);
            t = -4.0 * t * t + 4.0 * t;
            sphereCenter.y += t * 0.7;
            
            sphereCenter.x += sin(iTime) * 2.0;
            sphereCenter.z += cos(iTime) * 2.0;
        }
             
        if(i == 1)
        {
            float t = fract(iTime*0.47);
            t = -4.0 * t * t + 4.0 * t;
            sphereCenter.y += t * 1.7;
            
            sphereCenter.x += sin(iTime+3.14) * 2.0;
            sphereCenter.z += cos(iTime+3.14) * 2.0;
        }             
        //---------------------------------------
                
        float resultRayLength = raySphereIntersect(rayPos,rayDir,sphereCenter,sphereRadius);
        if(resultRayLength < hitData.rayLength && resultRayLength > 0.001)
        {
            //if a shorter(better) hit ray found, update
            hitData.rayLength = resultRayLength;
            vec3 hitPos = rayPos + rayDir * resultRayLength;
        hitData.normal = normalize(hitPos - sphereCenter);
        }
    }
    
    //all test finished, return shortest(best) hit data
    return hitData;
}
//--random functions-------------------------------------------------------------------
float rand01(float seed) { return fract(sin(seed)*43758.5453123); }
vec3 randomInsideUnitSphere(vec3 rayDir,vec3 rayPos, float extraSeed)
{
    return vec3(rand01(iTime * (rayDir.x + rayPos.x + 0.357) * extraSeed),
                rand01(iTime * (rayDir.y + rayPos.y + 16.35647) *extraSeed),
                rand01(iTime * (rayDir.z + rayPos.z + 425.357) * extraSeed));
}
//---------------------------------------------------------------------
vec4 calculateFinalColor(vec3 cameraPos, vec3 cameraRayDir, float AAIndex)
{
    //init
    vec3 finalColor = vec3(0.0);
    float absorbMul = 1.0;
    vec3 rayStartPos = cameraPos;
    vec3 rayDir = cameraRayDir;
    
    //only for CineShader, to show depth
    float firstHitRayLength = -1.0;
    
    //can't write recursive function in GLSL, so write it in a for loop
    //will loop until hitting any light source / bounces too many times
    for(int i = 0; i < MAX_BOUNCE; i++)
    {
        HitData h = AllObjectsRayTest(rayStartPos + rayDir * 0.0001,rayDir);//+0.0001 to prevent ray already hit at start pos
        
        //only for CineShader, to show depth
        firstHitRayLength = firstHitRayLength < 0.0 ? h.rayLength : firstHitRayLength;
               
        //if ray can't hit anything, rayLength will remain default value 9999.0
        //which enters this if()
        //** 99999 is too large for mobile, use 9900 as threshold now **
        if(h.rayLength >= 9900.0)
        {
            vec3 skyColor = vec3(0.7,0.85,1.0);//hit nothing = hit sky color
            finalColor = skyColor * absorbMul;
            break;
        }   
               
    absorbMul *= 0.8; //every bounce absorb some light(more bounces = darker)
        
        //update rayStartPos for next bounce
    rayStartPos = rayStartPos + rayDir * h.rayLength; 
        //update rayDir for next bounce
        float rougness = 0.05 + iMouse.x / iResolution.x; //hardcode "drag the window LR to control roughness"
    rayDir = normalize(reflect(rayDir,h.normal) + randomInsideUnitSphere(rayDir,rayStartPos,AAIndex) * rougness);       
    }
    
    return vec4(finalColor,firstHitRayLength);//alpha nly for CineShader, to show depth
}
//-----------------------------------------------------------------------
void main() {
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = fragCoord/iResolution.xy;
    
  uv = uv * 2.0 - 1.0;//transform from [0,1] to [-1,1]
    uv.x *= iResolution.x / iResolution.y; //aspect fix

    vec3 cameraPos = vec3(sin(iTime * 0.47) * 4.0,sin(iTime * 0.7)*8.0+6.0,-25.0);//camera pos animation
    vec3 cameraFocusPoint = vec3(0,0.0 + sin(iTime),0);//camera look target point animation
    vec3 cameraDir = normalize(cameraFocusPoint - cameraPos);
    
    //TEMPCODE: fov & all ray init dir, it is wrong!!!!
    //----------------------------------------------------
    float fovTempMul = 0.2 + sin(iTime * 0.4) * 0.05;//fov animation
    vec3 rayDir = normalize(cameraDir + vec3(uv,0) * fovTempMul);
    //----------------------------------------------------

    vec4 finalColor = vec4(0);
    for(int i = 1; i <= AVERAGECOUNT; i++)
    {
        finalColor+= calculateFinalColor(cameraPos,rayDir, float(i));
    }
    finalColor = finalColor/float(AVERAGECOUNT);//brute force AA & denoise
    finalColor.rgb = pow(finalColor.rgb,vec3(1.0/2.2));//gamma correction
    
    //only for CineShader, to show depth
    float z = finalColor.w; //z is linear world space distance from camera to surface
    float cineShaderZ; //expect 0~1
    cineShaderZ = pow(clamp(1.0 - max(0.0,z-21.0) * (1.0/6.0),0.0,1.0),2.0);
    
    //result
    fragColor = vec4(finalColor.rgb,cineShaderZ);
})";

int main()
{
    int width = 800;
    int height = 600;

    glm::vec2 screen(width, height);

    float deltaTime = 0.0f; 
    float lastFrame = 0.0f; 

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(width, height, "OpenglContext", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "failed to create window" << std::endl;
        exit(-1);
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "failed to initialize glad with processes " << std::endl;
        exit(-1);
    }

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    int samples = 4;
    float quadVerts[] = {
        -1.0, -1.0,     0.0, 0.0,
        -1.0, 1.0,      0.0, 1.0,
        1.0, -1.0,      1.0, 0.0,

        1.0, -1.0,      1.0, 0.0,
        -1.0, 1.0,      0.0, 1.0,
        1.0, 1.0,       1.0, 1.0
    };

    GLuint VAO;
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
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glUseProgram(shaderProgram);
    glUniform2fv(glGetUniformLocation(shaderProgram, "iResolution"), 1, &screen[0]);


    while (!glfwWindowShouldClose(window))
    {

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame; 

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, true);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);


        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUseProgram(shaderProgram);
        glUniform1f(glGetUniformLocation(shaderProgram, "iTime"), (int)currentFrame % 60); 
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);


        glfwSwapBuffers(window);
        glfwPollEvents();

    }
    glfwTerminate();
    // cleanup
}