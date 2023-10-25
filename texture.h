#ifndef TEXTURE_H
#define TEXTURE_H

#include "common.h"

class Texture
{
public:
    Texture(GLenum target, GLenum wrapMode, GLenum filterMode, bool vFilp, bool sRGB);
    Texture(GLenum target, const std::string &filename);
    ~Texture();

    void loadFromFile(const char *filename);
    void loadFromePixels(GLubyte *pixels, GLint w, GLint h, GLint depth, GLint channels, bool isFloat);

    void createEmpty(GLint w, GLint h);

    void bindToChannel(GLint channel);
    void release();

    static bool savePixelsToFile(const char *filepath, GLubyte *pixels, GLint w, GLint h, int depth);

    GLint width() const { return mWidth; }
    GLint height() const { return mHeight; }
    GLint depth() const { return mDepth; }

    bool isLoaded() const { return mLoaded; }

    GLuint textureId() const { return mTextureId; }

protected:
    void loadPixels(GLubyte *pixels, GLint w, GLint h, GLint depth, GLint channels, bool isFloat, GLint cubemapLayer);

private:
    GLenum mTarget;
    GLenum mFilterMode;
    GLenum mWrapMode;

    GLuint mTextureId;
    GLint mWidth, mHeight, mDepth;

    bool mVFlip;
    bool msRGB;
    bool mLoaded;
};

#endif // GL_TEXTURE_H
