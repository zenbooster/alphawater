#ifndef TEXTURE_H
#define TEXTURE_H

#include "common.h"

class Texture
{
private:
    GLenum mTarget;
    GLenum mWrapMode;
	GLenum mFilterMode;
    bool mVFlip;
    //bool msRGB;
    GLuint mTextureId;
	bool mLoaded;

    GLint mWidth, mHeight, mDepth;
	
	void set_params(void);

protected:
    void loadPixels(GLubyte *pixels, GLint w, GLint h, GLint depth, GLint channels, bool isFloat, GLint cubemapLayer);

public:
	enum TEnumResizeContent
	{
		ercNone,
		ercFromBottomLeft,
		ercFromCenter
	};

    Texture(GLenum target, GLenum wrapMode, GLenum filterMode, bool vFlip);//, bool sRGB);
    Texture(GLenum target, const std::string &filename);
    ~Texture();

    void loadFromFile(const char *filename);
    void loadFromePixels(GLubyte *pixels, GLint w, GLint h, GLint depth, GLint channels, bool isFloat);

    void createEmpty(GLint w, GLint h);
	void resize(int w, int h, TEnumResizeContent erc = ercNone);

    void bindToChannel(GLint channel);
    void release();

    static bool savePixelsToFile(const char *filepath, GLubyte *pixels, GLint w, GLint h, int depth);

    GLint width() const { return mWidth; }
    GLint height() const { return mHeight; }
    GLint depth() const { return mDepth; }

    bool isLoaded() const { return mLoaded; }

    GLuint textureId() const { return mTextureId; }
};

#endif // GL_TEXTURE_H
