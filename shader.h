#ifndef SHADER_H
#define SHADER_H

#include "common.h"

enum ShaderType {
    Vertex = 1 << 1,
    Fragment = 1 << 2
};

class Shader
{
public:
    typedef enum ShaderType ShaderType;

    Shader(Shader::ShaderType shaderType);
    ~Shader();

    bool compileSourceCode(const char *source);
    bool compileSourceCode(const string &source);
    bool compileSourceFile(const string &filename);

    bool isCompiled() const { return mCompiled; }

    GLuint shaderId() const { return mShaderId; }
    Shader::ShaderType shaderType() const { return mShaderType; }

    string sourceCode() const { return mSourceCode; }
    string log() const { return mLog; }

protected:
    bool create();
    void destroy();
    bool compile(const char *source);

private:
    friend class ShaderProgram;

    bool mCompiled;
    GLuint mShaderId;
    ShaderType mShaderType;

    string mLog;
    string mSourceCode;
};
#endif