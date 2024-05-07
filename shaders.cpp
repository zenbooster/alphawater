#include "shaders.h"

const char *vertexShader =
	"#version 330 core\n"
    "precision mediump float;\n"
    "precision mediump int;\n"
	"layout (location = 0) in vec3 vPosition;\n"
    "layout (location = 1) in vec2 vUV;\n"
    "void main() {\n"
    "    gl_Position = vec4(vPosition, 1.0f);\n"
    "}";

const char *fragmentShaderPassHeader =	
    "#version 330 core\n"
    "precision mediump float;\n"
    "precision mediump int;\n"
	"uniform vec3       iResolution;\n"
    "uniform float      iGlobalTime;\n"
    "uniform float      iTime;\n"
    "uniform float      iChannelTime[4];\n"
    "uniform vec4       iMouse;\n"
    "uniform vec4       iDate;\n"
    "uniform float      iSampleRate;\n"
    "uniform vec3       iChannelResolution[4];\n"
    "uniform int        iFrame;\n"
    "uniform float      iTimeDelta;\n"
    "uniform float      iFrameRate;\n"
    "struct Channel {\n"
    "   vec3    resolution;\n"
    "   float   time;\n"
    "};\n";
//    "uniform Channel iChannel[4];\n";

const char *fragmentShaderPassFooter =
    "out vec4 fragColor;\n"
    "void main() {\n"
	"   vec4 color;\n"
    "   mainImage(color, gl_FragCoord.xy);\n"
	"   fragColor = color;\n"
    "}";
