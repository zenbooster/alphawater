#include "common.h"
#include "shaderprogram.h"
//#include "g_math.h"
/*#include "vector2.h"
#include "vector3.h"
#include "vector3.h"
#include "matrix2x2.h"
#include "matrix2x3.h"
#include "matrix2x4.h"
#include "matrix2x4.h"
#include "matrix3x2.h"
#include "matrix3x3.h"
#include "matrix3x4.h"
#include "matrix4x2.h"
#include "matrix4x3.h"
#include "matrix4x4.h"
*/
#include "common.h"

ShaderProgram::ShaderProgram()
    : mProgramId(0),
      mVertexShader(nullptr),
      mFragmentShader(nullptr)
{
}

ShaderProgram::~ShaderProgram()
{
}

bool ShaderProgram::addShader(Shader *shader)
{
   if (shader->shaderType() == Shader::ShaderType::Vertex)
   {
       if (mVertexShader != nullptr)
           return false;
       mVertexShader = shader;
   }
   else if (shader->shaderType() == Shader::ShaderType::Fragment)
   {
       if (mFragmentShader != nullptr)
           return false;
       mFragmentShader = shader;
   }

    return true;
}

bool ShaderProgram::addShaderFromSource(Shader::ShaderType type, const char *source)
{
    Shader *shader = new Shader(type);

    if (shader->compileSourceCode(source))
    {
        if (shader->isCompiled())
        {
            if (addShader(shader))
                return true;
            else
            {
                shader->destroy();
                delete shader;
                return false;
            }
        }
        else
        {
			log4cplus::Logger logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("app"));
			LOG4CPLUS_WARN(logger, LOG4CPLUS_TEXT("Compile Shader Error: ") << shader->log());
            shader->destroy();
            delete shader;
            return false;
        }
    }
    else
    {
        shader->destroy();
        delete shader;
        return false;
    }

    return true;
}

bool ShaderProgram::addShaderFromSource(Shader::ShaderType type, const std::string &source)
{
    return addShaderFromSource(type, source.c_str());
}

bool ShaderProgram::addShaderFromFile(Shader::ShaderType type, const std::string &filename)
{
    Shader *shader = new Shader(type);

    if (shader->compileSourceFile(filename))
    {
        if (shader->isCompiled())
        {
            if (addShader(shader))
                return true;
            else
            {
                shader->destroy();
                delete shader;
                return false;
            }
        }
        else
        {
			log4cplus::Logger logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("app"));
			LOG4CPLUS_WARN(logger, LOG4CPLUS_TEXT("") << shader->log());
            shader->destroy();
            delete shader;
            return false;
        }
    }
    else
    {
        shader->destroy();
        delete shader;
        return false;
    }

    return true;
}

void ShaderProgram::removeAllShader()
{
    if (mProgramId && mVertexShader && mFragmentShader)
    {
        glDetachShader(mProgramId, mVertexShader->shaderId());
        mVertexShader->destroy();
        delete mVertexShader;

        glDetachShader(mProgramId, mFragmentShader->shaderId());
        mFragmentShader->destroy();
        delete mFragmentShader;

        mLinked = false;
    }
}

bool ShaderProgram::create()
{
    mProgramId = glCreateProgram();

    if (!mProgramId)
    {
		log4cplus::Logger logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("app"));
		LOG4CPLUS_WARN(logger, LOG4CPLUS_TEXT("Could not create program object:"));
        return false;
    }

    return true;
}

bool ShaderProgram::link()
{
    if (!create())
        return false;

    if (!mVertexShader || !mFragmentShader)
        return false;

    glAttachShader(mProgramId, mVertexShader->shaderId());
    glAttachShader(mProgramId, mFragmentShader->shaderId());
    glLinkProgram(mProgramId);

    int value = 0;
    glGetProgramiv(mProgramId, GL_LINK_STATUS, &value);
    mLinked = (value != 0);

    if (!mLinked)
    {
        int length = 0;
        glGetProgramiv(mProgramId, GL_INFO_LOG_LENGTH, &value);
        if (value > 1)
        {
            mLog.resize(value);
            glGetProgramInfoLog(mProgramId, value, &length, &mLog[0]);
			
			log4cplus::Logger logger = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT("app"));
			LOG4CPLUS_WARN(logger, LOG4CPLUS_TEXT("ShaderProgram::link: " << mLog));
        }

        removeAllShader();
    }

    return mLinked;
}

bool ShaderProgram::bind()
{
    if (!mProgramId || !mLinked)
        return false;

    glUseProgram(mProgramId);
    return true;
}

void ShaderProgram::release()
{
    glUseProgram(0);
}

GLint ShaderProgram::attributeLocation(const char *name)
{
    return glGetAttribLocation(mProgramId, name);;
}

GLint ShaderProgram::attributeLocation(const std::string &name)
{
    return glGetAttribLocation(mProgramId, name.c_str());
}

void ShaderProgram::bindAttributeLocation(const char *name, int location)
{
    glBindAttribLocation(mProgramId, location, name);
}

void ShaderProgram::bindAttributeLocation(const std::string &name, int location)
{
    glBindAttribLocation(mProgramId, location, name.c_str());
}

void ShaderProgram::disableAttributeArray(int location)
{
    glDisableVertexAttribArray(location);
}

void ShaderProgram::disableAttributeArray(const char *name)
{
    int location = glGetAttribLocation(mProgramId, name);
    glDisableVertexAttribArray(location);
}

void ShaderProgram::disableAttributeArray(const std::string &name)
{
    int location = glGetAttribLocation(mProgramId, name.c_str());
    glDisableVertexAttribArray(location);
}

void ShaderProgram::enableAttributeArray(int location)
{
    glEnableVertexAttribArray(location);
}

void ShaderProgram::enableAttributeArray(const char *name)
{
    int location = glGetAttribLocation(mProgramId, name);
    glEnableVertexAttribArray(location);
}

void ShaderProgram::enableAttributeArray(const std::string &name)
{
    int location = glGetAttribLocation(mProgramId, name.c_str());
    glEnableVertexAttribArray(location);
}

void ShaderProgram::setAttributeArray(int location, const GLfloat *values, int tupleSize, int stride)
{
    glVertexAttribPointer(location, tupleSize, GL_FLOAT, GL_FALSE, stride, values);
}

/*void ShaderProgram::setAttributeArray(int location, const Vector2 *values, int stride)
{
    glVertexAttribPointer(location, 2, GL_FLOAT, GL_FALSE, stride, values);
}

void ShaderProgram::setAttributeArray(int location, const Vector3 *values, int stride)
{
    glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, stride, values);
}

void ShaderProgram::setAttributeArray(int location, const Vector4 *values, int stride)
{
    glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE, stride, values);
}*/

void ShaderProgram::setAttributeArray(int location, GLenum type, const void *values, int tupleSize, int stride)
{
    glVertexAttribPointer(location, tupleSize, type, GL_TRUE, stride, values);
}

void ShaderProgram::setAttributeBuffer(int location, GLenum type, int offset, int tupleSize, int stride)
{
    glVertexAttribPointer(location, tupleSize, type, GL_TRUE, stride,
                          reinterpret_cast<const void *>(intptr_t(offset)));
}

void ShaderProgram::setAttributeArray(const char *name, const GLfloat *values, int tupleSize, int stride)
{
    GLint location = glGetAttribLocation(mProgramId, name);;
    glVertexAttribPointer(location, tupleSize, GL_FLOAT, GL_FALSE, stride, values);
}

/*void ShaderProgram::setAttributeArray(const char *name, const Vector2 *values, int stride)
{
    GLint location = glGetAttribLocation(mProgramId, name);;
    glVertexAttribPointer(location, 2, GL_FLOAT, GL_FALSE, stride, values);
}

void ShaderProgram::setAttributeArray(const char *name, const Vector3 *values, int stride)
{
    GLint location = glGetAttribLocation(mProgramId, name);;
    glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, stride, values);
}

void ShaderProgram::setAttributeArray(const char *name, const Vector4 *values, int stride)
{
    GLint location = glGetAttribLocation(mProgramId, name);;
    glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE, stride, values);
}*/

void ShaderProgram::setAttributeArray(const char *name, GLenum type, const void *values, int tupleSize, int stride)
{
    GLint location = glGetAttribLocation(mProgramId, name);;
    glVertexAttribPointer(location, tupleSize, type, GL_TRUE, stride, values);
}

void ShaderProgram::setAttributeBuffer(const char *name, GLenum type, int offset, int tupleSize, int stride)
{
    GLint location = glGetAttribLocation(mProgramId, name);;
    glVertexAttribPointer(location, tupleSize, type, GL_TRUE, stride,
                              reinterpret_cast<const void *>(intptr_t(offset)));
}

void ShaderProgram::setAttributeValue(int location, const GLfloat value)
{
    glVertexAttrib1f(location, value);
}

void ShaderProgram::setAttributeValue(int location, const GLfloat x, const GLfloat y)
{
    glVertexAttrib2f(location, x, y);
}

void ShaderProgram::setAttributeValue(int location, const GLfloat x, const GLfloat y, const GLfloat z)
{
    glVertexAttrib3f(location, x, y, z);
}

void ShaderProgram::setAttributeValue(int location, const GLfloat x, const GLfloat y, const GLfloat z, const GLfloat w)
{
    glVertexAttrib4f(location, x, y, z, w);
}

/*void ShaderProgram::setAttributeValue(int location, const Vector2 &value)
{
    const GLfloat *fv = reinterpret_cast<const GLfloat *>(&value);
    glVertexAttrib2fv(location, fv);
}

void ShaderProgram::setAttributeValue(int location, const Vector3 &value)
{
    const GLfloat *fv = reinterpret_cast<const GLfloat *>(&value);
    glVertexAttrib3fv(location, fv);
}

void ShaderProgram::setAttributeValue(int location, const Vector4 &value)
{
    const GLfloat *fv = reinterpret_cast<const GLfloat *>(&value);
    glVertexAttrib4fv(location, fv);
}*/

void ShaderProgram::setAttributeValue(const char *name, const GLfloat value)
{
    GLint location = glGetAttribLocation(mProgramId, name);
    glVertexAttrib1f(location, value);
}

void ShaderProgram::setAttributeValue(const char *name, const GLfloat x, const GLfloat y)
{
    GLint location = glGetAttribLocation(mProgramId, name);
    glVertexAttrib2f(location, x, y);
}

void ShaderProgram::setAttributeValue(const char *name, const GLfloat x, const GLfloat y, const GLfloat z)
{
    GLint location = glGetAttribLocation(mProgramId, name);
    glVertexAttrib3f(location, x, y, z);
}

void ShaderProgram::setAttributeValue(const char *name, const GLfloat x, const GLfloat y, const GLfloat z, const GLfloat w)
{
    GLint location = glGetAttribLocation(mProgramId, name);
    glVertexAttrib4f(location, x, y, z, w);
}

/*void ShaderProgram::setAttributeValue(const char *name, const Vector2 &value)
{
    GLint location = glGetAttribLocation(mProgramId, name);
    const GLfloat *fv = reinterpret_cast<const GLfloat *>(&value);
    glVertexAttrib2fv(location, fv);
}

void ShaderProgram::setAttributeValue(const char *name, const Vector3 &value)
{
    GLint location = glGetAttribLocation(mProgramId, name);
    const GLfloat *fv = reinterpret_cast<const GLfloat *>(&value);
    glVertexAttrib3fv(location, fv);
}

void ShaderProgram::setAttributeValue(const char *name, const Vector4 &value)
{
    GLint location = glGetAttribLocation(mProgramId, name);
    const GLfloat *fv = reinterpret_cast<const GLfloat *>(&value);
    glVertexAttrib4fv(location, fv);
}*/


void ShaderProgram::setUniformValue(int location, GLint value)
{
    glUniform1i(location, value);
}

void ShaderProgram::setUniformValue(int location, GLuint value)
{
    glUniform1i(location, value);
}

void ShaderProgram::setUniformValue(int location, const GLfloat value)
{
    glUniform1f(location, value);
}

void ShaderProgram::setUniformValue(int location, const GLfloat x, const GLfloat y)
{
    glUniform2f(location, x, y);
}

void ShaderProgram::setUniformValue(int location, const GLfloat x, const GLfloat y, const GLfloat z)
{
    glUniform3f(location, x, y, z);
}

void ShaderProgram::setUniformValue(int location, const GLfloat x, const GLfloat y, const GLfloat z, const GLfloat w)
{
    glUniform4f(location, x, y, z, w);
}

void ShaderProgram::setUniformValue(int location, const glm::vec2 &value)
{
    const GLfloat *fv = reinterpret_cast<const GLfloat *>(&value);
    glUniform2fv(location, 1, fv);
}

void ShaderProgram::setUniformValue(int location, const glm::vec3 &value)
{
    const GLfloat *fv = reinterpret_cast<const GLfloat *>(&value);
    glUniform3fv(location, 1, fv);
}

/*void ShaderProgram::setUniformValue(int location, const Vector4 &value)
{
    const GLfloat *fv = reinterpret_cast<const GLfloat *>(&value);
    glUniform4fv(location, 1, fv);
}

void ShaderProgram::setUniformValue(int location, const Matrix2x2 &value)
{
    const GLfloat *fv = reinterpret_cast<const GLfloat *>(&value);
    glUniformMatrix2fv(location, 1, GL_FALSE, fv);
}

void ShaderProgram::setUniformValue(int location, const Matrix2x3 &value)
{
    const GLfloat *fv = reinterpret_cast<const GLfloat *>(&value);
    glUniformMatrix2x3fv(location, 1, GL_FALSE, fv);
}

void ShaderProgram::setUniformValue(int location, const Matrix2x4 &value)
{
    const GLfloat *fv = reinterpret_cast<const GLfloat *>(&value);
    glUniformMatrix2x4fv(location, 1, GL_FALSE, fv);
}

void ShaderProgram::setUniformValue(int location, const Matrix3x2 &value)
{
    const GLfloat *fv = reinterpret_cast<const GLfloat *>(&value);
    glUniformMatrix3x2fv(location, 1, GL_FALSE, fv);
}

void ShaderProgram::setUniformValue(int location, const Matrix3x3 &value)
{
    const GLfloat *fv = reinterpret_cast<const GLfloat *>(&value);
    glUniformMatrix3fv(location, 1, GL_FALSE, fv);
}

void ShaderProgram::setUniformValue(int location, const Matrix3x4 &value)
{
    const GLfloat *fv = reinterpret_cast<const GLfloat *>(&value);
    glUniformMatrix3x4fv(location, 1, GL_FALSE, fv);
}

void ShaderProgram::setUniformValue(int location, const Matrix4x2 &value)
{
    const GLfloat *fv = reinterpret_cast<const GLfloat *>(&value);
    glUniformMatrix4x2fv(location, 1, GL_FALSE, fv);
}

void ShaderProgram::setUniformValue(int location, const Matrix4x3 &value)
{
    const GLfloat *fv = reinterpret_cast<const GLfloat *>(&value);
    glUniformMatrix4x3fv(location, 1, GL_FALSE, fv);
}

void ShaderProgram::setUniformValue(int location, const Matrix4x4 &value)
{
    const GLfloat *fv = reinterpret_cast<const GLfloat *>(&value);
    glUniformMatrix4fv(location, 1, GL_FALSE, fv);
}*/

void ShaderProgram::setUniformValue(const char *name, GLint value)
{
   GLint location = glGetUniformLocation(mProgramId, name);
   glUniform1i(location, value);
}

void ShaderProgram::setUniformValue(const char *name, GLuint value)
{
    GLint location = glGetUniformLocation(mProgramId, name);
    glUniform1i(location, value);
}

void ShaderProgram::setUniformValue(const char *name, const GLfloat value)
{
    GLint location = glGetUniformLocation(mProgramId, name);
    glUniform1f(location, value);
}

void ShaderProgram::setUniformValue(const char *name, const GLfloat x, const GLfloat y)
{
    GLint location = glGetUniformLocation(mProgramId, name);
    glUniform2f(location, x, y);
}

void ShaderProgram::setUniformValue(const char *name, const GLfloat x, const GLfloat y, const GLfloat z)
{
    GLint location = glGetUniformLocation(mProgramId, name);
    glUniform3f(location, x, y, z);
}

void ShaderProgram::setUniformValue(const char *name, const GLfloat x, const GLfloat y, const GLfloat z, const GLfloat w)
{
    GLint location = glGetUniformLocation(mProgramId, name);
    glUniform4f(location, x, y, z, w);
}

void ShaderProgram::setUniformValue(const char *name, const glm::vec2 &value)
{
    GLint location = glGetUniformLocation(mProgramId, name);
    const GLfloat *fv = reinterpret_cast<const GLfloat *>(&value);
    glUniform2fv(location, 1, fv);
}

void ShaderProgram::setUniformValue(const char *name, const glm::vec3 &value)
{
    GLint location = glGetUniformLocation(mProgramId, name);
    const GLfloat *fv = reinterpret_cast<const GLfloat *>(&value);
    glUniform3fv(location, 1, fv);
}

/*void ShaderProgram::setUniformValue(const char *name, const Vector4 &value)
{
    GLint location = glGetUniformLocation(mProgramId, name);
    const GLfloat *fv = reinterpret_cast<const GLfloat *>(&value);
    glUniform4fv(location, 1, fv);
}

void ShaderProgram::setUniformValue(const char *name, const Matrix2x2 &value)
{
    GLint location = glGetUniformLocation(mProgramId, name);
    glUniformMatrix2fv(location, 1, GL_FALSE, value.constData());
}

void ShaderProgram::setUniformValue(const char *name, const Matrix2x3 &value)
{
    GLint location = glGetUniformLocation(mProgramId, name);
    glUniformMatrix2x3fv(location, 1, GL_FALSE, value.constData());
}

void ShaderProgram::setUniformValue(const char *name, const Matrix2x4 &value)
{
    GLint location = glGetUniformLocation(mProgramId, name);
    glUniformMatrix2x4fv(location, 1, GL_FALSE, value.constData());
}

void ShaderProgram::setUniformValue(const char *name, const Matrix3x2 &value)
{
    GLint location = glGetUniformLocation(mProgramId, name);
    glUniformMatrix3x2fv(location, 1, GL_FALSE, value.constData());
}

void ShaderProgram::setUniformValue(const char *name, const Matrix3x3 &value)
{
    GLint location = glGetUniformLocation(mProgramId, name);
    glUniformMatrix3fv(location, 1, GL_FALSE, value.constData());
}

void ShaderProgram::setUniformValue(const char *name, const Matrix3x4 &value)
{
    GLint location = glGetUniformLocation(mProgramId, name);
    glUniformMatrix3x4fv(location, 1, GL_FALSE, value.constData());
}

void ShaderProgram::setUniformValue(const char *name, const Matrix4x2 &value)
{
    GLint location = glGetUniformLocation(mProgramId, name);
    glUniformMatrix4x2fv(location, 1, GL_FALSE, value.constData());
}

void ShaderProgram::setUniformValue(const char *name, const Matrix4x3 &value)
{
    GLint location = glGetUniformLocation(mProgramId, name);
    glUniformMatrix4x3fv(location, 1, GL_FALSE, value.constData());
}

void ShaderProgram::setUniformValue(const char *name, const Matrix4x4 &value)
{
    GLint location = glGetUniformLocation(mProgramId, name);
    glUniformMatrix4fv(location, 1, GL_FALSE, value.constData());
}*/

void ShaderProgram::setUniformValueArray(int location, const GLint *values, int count)
{
    glUniform1iv(location, count, values);
}

void ShaderProgram::setUniformValueArray(int location, const GLuint *values, int count)
{
    const GLint *iv = reinterpret_cast<const GLint *>(values);
    glUniform1iv(location, count, iv);
}

void ShaderProgram::setUniformValueArray(int location, const GLfloat *values, int count, int tupleSize)
{
    if (tupleSize == 1)
        glUniform1fv(location, count, values);
    else if (tupleSize == 2)
        glUniform2fv(location, count, values);
    else if (tupleSize == 3)
        glUniform3fv(location, count, values);
    else if (tupleSize == 4)
        glUniform4fv(location, count, values);
}

/*void ShaderProgram::setUniformValueArray(int location, const Vector2 *values, int count)
{
    const GLfloat *fv = reinterpret_cast<const GLfloat *>(values);
    glUniform2fv(location, count, fv);
}

void ShaderProgram::setUniformValueArray(int location, const Vector3 *values, int count)
{
    const GLfloat *fv = reinterpret_cast<const GLfloat *>(values);
    glUniform3fv(location, count, fv);
}

void ShaderProgram::setUniformValueArray(int location, const Vector4 *values, int count)
{
    const GLfloat *fv = reinterpret_cast<const GLfloat *>(values);
    glUniform4fv(location, count, fv);
}

void ShaderProgram::setUniformValueArray(int location, const Matrix2x2 *values, int count)
{
    const GLfloat *fv = reinterpret_cast<const GLfloat *>(values);
    glUniformMatrix2fv(location, count, GL_FALSE, fv);
}

void ShaderProgram::setUniformValueArray(int location, const Matrix2x3 *values, int count)
{
    const GLfloat *fv = reinterpret_cast<const GLfloat *>(values);
    glUniformMatrix2x3fv(location, count, GL_FALSE, fv);
}

void ShaderProgram::setUniformValueArray(int location, const Matrix2x4 *values, int count)
{
    const GLfloat *fv = reinterpret_cast<const GLfloat *>(values);
    glUniformMatrix2x4fv(location, count, GL_FALSE, fv);
}

void ShaderProgram::setUniformValueArray(int location, const Matrix3x2 *values, int count)
{
    const GLfloat *fv = reinterpret_cast<const GLfloat *>(values);
    glUniformMatrix3x2fv(location, count, GL_FALSE, fv);
}

void ShaderProgram::setUniformValueArray(int location, const Matrix3x3 *values, int count)
{
    const GLfloat *fv = reinterpret_cast<const GLfloat *>(values);
    glUniformMatrix3fv(location, count, GL_FALSE, fv);
}

void ShaderProgram::setUniformValueArray(int location, const Matrix3x4 *values, int count)
{
    const GLfloat *fv = reinterpret_cast<const GLfloat *>(values);
    glUniformMatrix3x4fv(location, count, GL_FALSE, fv);
}

void ShaderProgram::setUniformValueArray(int location, const Matrix4x2 *values, int count)
{
    const GLfloat *fv = reinterpret_cast<const GLfloat *>(values);
    glUniformMatrix4x2fv(location, count, GL_FALSE, fv);
}

void ShaderProgram::setUniformValueArray(int location, const Matrix4x3 *values, int count)
{
    const GLfloat *fv = reinterpret_cast<const GLfloat *>(values);
    glUniformMatrix4x3fv(location, count, GL_FALSE, fv);
}

void ShaderProgram::setUniformValueArray(int location, const Matrix4x4 *values, int count)
{
    const GLfloat *fv = reinterpret_cast<const GLfloat *>(values);
    glUniformMatrix4fv(location, count, GL_FALSE, fv);
}*/

void ShaderProgram::setUniformValueArray(const char *name, const GLint *values, int count)
{
     GLint location = glGetUniformLocation(mProgramId, name);
     glUniform1iv(location, count, values);
}

void ShaderProgram::setUniformValueArray(const char *name, const GLuint *values, int count)
{
    GLint location = glGetUniformLocation(mProgramId, name);
    const GLint *iv = reinterpret_cast<const GLint *>(values);
    glUniform1iv(location, count, iv);
}

void ShaderProgram::setUniformValueArray(const char *name, const GLfloat *values, int count, int tupleSize)
{
    GLint location = glGetUniformLocation(mProgramId, name);

    if (tupleSize == 1)
        glUniform1fv(location, count, values);
    else if (tupleSize == 2)
        glUniform2fv(location, count, values);
    else if (tupleSize == 3)
        glUniform3fv(location, count, values);
    else if (tupleSize == 4)
        glUniform4fv(location, count, values);
}

/*void ShaderProgram::setUniformValueArray(const char *name, const Vector2 *values, int count)
{
    GLint location = glGetUniformLocation(mProgramId, name);
    const GLfloat *fv = reinterpret_cast<const GLfloat *>(values);
    glUniform2fv(location, count, fv);
}

void ShaderProgram::setUniformValueArray(const char *name, const Vector3 *values, int count)
{
    GLint location = glGetUniformLocation(mProgramId, name);
    const GLfloat *fv = reinterpret_cast<const GLfloat *>(values);
    glUniform3fv(location, count, fv);
}

void ShaderProgram::setUniformValueArray(const char *name, const Vector4 *values, int count)
{
    GLint location = glGetUniformLocation(mProgramId, name);
    const GLfloat *fv = reinterpret_cast<const GLfloat *>(values);
    glUniform4fv(location, count, fv);
}

void ShaderProgram::setUniformValueArray(const char *name, const Matrix2x3 *values, int count)
{
    GLint location = glGetUniformLocation(mProgramId, name);
    const GLfloat *fv = reinterpret_cast<const GLfloat *>(values);
    glUniformMatrix2x3fv(location, count, GL_FALSE, fv);
}

void ShaderProgram::setUniformValueArray(const char *name, const Matrix2x4 *values, int count)
{
    GLint location = glGetUniformLocation(mProgramId, name);
    const GLfloat *fv = reinterpret_cast<const GLfloat *>(values);
    glUniformMatrix2x4fv(location, count, GL_FALSE, fv);
}

void ShaderProgram::setUniformValueArray(const char *name, const Matrix3x2 *values, int count)
{
    GLint location = glGetUniformLocation(mProgramId, name);
    const GLfloat *fv = reinterpret_cast<const GLfloat *>(values);
    glUniformMatrix3x2fv(location, count, GL_FALSE, fv);
}

void ShaderProgram::setUniformValueArray(const char *name, const Matrix3x3 *values, int count)
{
    GLint location = glGetUniformLocation(mProgramId, name);
    const GLfloat *fv = reinterpret_cast<const GLfloat *>(values);
    glUniformMatrix3fv(location, count, GL_FALSE, fv);
}

void ShaderProgram::setUniformValueArray(const char *name, const Matrix3x4 *values, int count)
{
    GLint location = glGetUniformLocation(mProgramId, name);
    const GLfloat *fv = reinterpret_cast<const GLfloat *>(values);
    glUniformMatrix3x4fv(location, count, GL_FALSE, fv);
}

void ShaderProgram::setUniformValueArray(const char *name, const Matrix4x2 *values, int count)
{
    GLint location = glGetUniformLocation(mProgramId, name);
    const GLfloat *fv = reinterpret_cast<const GLfloat *>(values);
    glUniformMatrix4x2fv(location, count, GL_FALSE, fv);
}

void ShaderProgram::setUniformValueArray(const char *name, const Matrix4x3 *values, int count)
{
    GLint location = glGetUniformLocation(mProgramId, name);
    const GLfloat *fv = reinterpret_cast<const GLfloat *>(values);
    glUniformMatrix4x3fv(location, count, GL_FALSE, fv);
}

void ShaderProgram::setUniformValueArray(const char *name, const Matrix4x4 *values, int count)
{
    GLint location = glGetUniformLocation(mProgramId, name);
    const GLfloat *fv = reinterpret_cast<const GLfloat *>(values);
    glUniformMatrix4fv(location, count, GL_FALSE, fv);
}*/

GLint ShaderProgram::uniformLocation(const char *name)
{
    return glGetUniformLocation(mProgramId, name);
}

GLint ShaderProgram::uniformLocation(const std::string &name)
{
    return glGetUniformLocation(mProgramId, name.c_str());
}
