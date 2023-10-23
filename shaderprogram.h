#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H

#include "shader.h"

/*class Vector2;
class Vector3;
class Vector4;
class Matrix2x2;
class Matrix2x3;
class Matrix2x4;
class Matrix3x2;
class Matrix3x3;
class Matrix3x4;
class Matrix4x2;
class Matrix4x3;
class Matrix4x4;
*/

class ShaderProgram
{
public:
    ShaderProgram();
    ~ShaderProgram();

    const Shader *vertexShader() const { return mVertexShader; }
    const Shader *fragmentShader() const { return mFragmentShader; }

    bool addShader(Shader *shader);
    bool addShaderFromSource(Shader::ShaderType type, const char *source);
    bool addShaderFromSource(Shader::ShaderType type, const std::string &source);
    bool addShaderFromFile(Shader::ShaderType type, const std::string &filename);

    void removeAllShader();

    bool create();
    bool link();
    bool bind();
    void release();

    bool isLinked() const { return mLinked; }
    const std::string &log() const { return mLog; }
    GLuint programId() const { return mProgramId; }

    GLint attributeLocation(const char *name);
    GLint attributeLocation(const std::string &name);

    void bindAttributeLocation(const char *name, int location);
    void bindAttributeLocation(const std::string &name, int location);

    void disableAttributeArray(int location);
    void disableAttributeArray(const char *name);
    void disableAttributeArray(const std::string &name);

    void enableAttributeArray(int location);
    void enableAttributeArray(const char *name);
    void enableAttributeArray(const std::string &name);

    void setAttributeArray(int location, const GLfloat *values, int tupleSize, int stride);
    /*void setAttributeArray(int location, const Vector2 *valus, int stride);
    void setAttributeArray(int location, const Vector3 *values, int stride);
    void setAttributeArray(int location, const Vector4 *values, int stride);
	*/
    void setAttributeArray(int location, GLenum type, const void *values, int tupleSize, int stride);

    void setAttributeBuffer(int location, GLenum type, int offset, int tupleSize, int stride);

    void setAttributeArray(const char *name, const GLfloat *values, int tupleSize, int stride);
	/*
    void setAttributeArray(const char *name, const Vector2 *valus, int stride);
    void setAttributeArray(const char *name, const Vector3 *values, int stride);
    void setAttributeArray(const char *name, const Vector4 *values, int stride);
	*/
    void setAttributeArray(const char *name, GLenum type, const void *values, int tupleSize, int stride);

    void setAttributeBuffer(const char *name, GLenum type, int offset, int tupleSize, int stride);

    void setAttributeValue(int location, GLfloat value);
    void setAttributeValue(int location, GLfloat x, GLfloat y);
    void setAttributeValue(int location, GLfloat x, GLfloat y, GLfloat z);
    void setAttributeValue(int location, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
    /*void setAttributeValue(int location, const Vector2 &value);
    void setAttributeValue(int location, const Vector3 &value);
    void setAttributeValue(int location, const Vector4 &value);
	*/

    void setAttributeValue(const char *name, GLfloat value);
    void setAttributeValue(const char *name, GLfloat x, GLfloat y);
    void setAttributeValue(const char *name, GLfloat x, GLfloat y, GLfloat z);
    void setAttributeValue(const char *name, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
    /*void setAttributeValue(const char *name, const Vector2 &value);
    void setAttributeValue(const char *name, const Vector3 &value);
    void setAttributeValue(const char *name, const Vector4 &value);
	*/

    void setUniformValue(int location, GLint value);
    void setUniformValue(int location, GLuint value);
    void setUniformValue(int location, GLfloat value);
    void setUniformValue(int location, GLfloat x, GLfloat y);
    void setUniformValue(int location, GLfloat x, GLfloat y, GLfloat z);
    void setUniformValue(int location, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
    /*void setUniformValue(int location, const Vector2 &value);
    void setUniformValue(int location, const Vector3 &value);
    void setUniformValue(int location, const Vector4 &value);
    void setUniformValue(int location, const Matrix2x2 &value);
    void setUniformValue(int location, const Matrix2x3 &value);
    void setUniformValue(int location, const Matrix2x4 &value);
    void setUniformValue(int location, const Matrix3x2 &value);
    void setUniformValue(int location, const Matrix3x3 &value);
    void setUniformValue(int location, const Matrix3x4 &value);
    void setUniformValue(int location, const Matrix4x2 &value);
    void setUniformValue(int location, const Matrix4x3 &value);
    void setUniformValue(int location, const Matrix4x4 &value);
	*/
    void setUniformValue(const char *name, GLint value);
    void setUniformValue(const char *name, GLuint value);
    void setUniformValue(const char *name, GLfloat value);
    void setUniformValue(const char *name, GLfloat x, GLfloat y);
    void setUniformValue(const char *name, GLfloat x, GLfloat y, GLfloat z);
    void setUniformValue(const char *name, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
    /*void setUniformValue(const char *name, const Vector2 &value);
    void setUniformValue(const char *name, const Vector3 &value);
    void setUniformValue(const char *name, const Vector4 &value);
    void setUniformValue(const char *name, const Matrix2x2 &value);
    void setUniformValue(const char *name, const Matrix2x3 &value);
    void setUniformValue(const char *name, const Matrix2x4 &value);
    void setUniformValue(const char *name, const Matrix3x2 &value);
    void setUniformValue(const char *name, const Matrix3x3 &value);
    void setUniformValue(const char *name, const Matrix3x4 &value);
    void setUniformValue(const char *name, const Matrix4x2 &value);
    void setUniformValue(const char *name, const Matrix4x3 &value);
    void setUniformValue(const char *name, const Matrix4x4 &value);
	*/
    void setUniformValueArray(int location, const GLint *values, int count);
    void setUniformValueArray(int location, const GLuint *values, int count);
    void setUniformValueArray(int location, const GLfloat *values, int count, int tupleSize);
    /*void setUniformValueArray(int location, const Vector2 *values, int count);
    void setUniformValueArray(int location, const Vector3 *values, int count);
    void setUniformValueArray(int location, const Vector4 *values, int count);
    void setUniformValueArray(int location, const Matrix2x2 *values, int count);
    void setUniformValueArray(int location, const Matrix2x3 *values, int count);
    void setUniformValueArray(int location, const Matrix2x4 *values, int count);
    void setUniformValueArray(int location, const Matrix3x2 *values, int count);
    void setUniformValueArray(int location, const Matrix3x3 *values, int count);
    void setUniformValueArray(int location, const Matrix3x4 *values, int count);
    void setUniformValueArray(int location, const Matrix4x2 *values, int count);
    void setUniformValueArray(int location, const Matrix4x3 *values, int count);
    void setUniformValueArray(int location, const Matrix4x4 *values, int count);
	*/
    void setUniformValueArray(const char *name, const GLint *values, int count);
    void setUniformValueArray(const char *name, const GLuint *values, int count);
    void setUniformValueArray(const char *name, const GLfloat *values, int count, int tupleSize);
    /*void setUniformValueArray(const char *name, const Vector2 *values, int count);
    void setUniformValueArray(const char *name, const Vector3 *values, int count);
    void setUniformValueArray(const char *name, const Vector4 *values, int count);
    void setUniformValueArray(const char *name, const Matrix2x2 *values, int count);
    void setUniformValueArray(const char *name, const Matrix2x3 *values, int count);
    void setUniformValueArray(const char *name, const Matrix2x4 *values, int count);
    void setUniformValueArray(const char *name, const Matrix3x2 *values, int count);
    void setUniformValueArray(const char *name, const Matrix3x3 *values, int count);
    void setUniformValueArray(const char *name, const Matrix3x4 *values, int count);
    void setUniformValueArray(const char *name, const Matrix4x2 *values, int count);
    void setUniformValueArray(const char *name, const Matrix4x3 *values, int count);
    void setUniformValueArray(const char *name, const Matrix4x4 *values, int count);
	*/
    GLint uniformLocation(const char *name);
    GLint uniformLocation(const std::string &name);

private:
    int mLinked;
    GLuint mProgramId;

    std::string mLog;
    Shader *mVertexShader;
    Shader *mFragmentShader;
};

#endif // GL_SHADER_PROGRAM_H
