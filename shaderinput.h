#ifndef SHADER_INPUT_H
#define SHADER_INPUT_H

#include <glm/glm.hpp>

class ShaderInput
{
public:
    glm::vec3   iResolution;
    float       iTime;
    float       iGlobalTime;
    glm::vec4   iMouse;
    glm::vec4   iDate;
    float       iSampleRate;
    glm::vec3   iChannelResolution[4];
    float       iChannelTime[4];

    int         iFrame;
    float       iTimeDelta;
    float       iFrameRate;
};

#endif // SHADERINPUT_H
