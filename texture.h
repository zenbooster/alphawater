#ifndef TEXTURE_H
#define TEXTURE_H

#include "common.h"
#include <memory>

class TextureIdHolder
{
private:
	GLuint mTextureId;

public:
	TextureIdHolder();
	~TextureIdHolder();
	
	GLuint textureId() const { return mTextureId; }
};

class Texture
{
private:
    GLenum mTarget;
    GLenum mWrapMode;
	GLenum mFilterMode;
    bool mVFlip;
    //bool msRGB;
	shared_ptr<TextureIdHolder> tih;
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

    void createEmpty(GLint w, GLint h, int i = 0);
	void resize(int w, int h, int i = 0, TEnumResizeContent erc = ercNone);

    void bindToChannel(GLint channel);
    void release();

    static bool savePixelsToFile(const char *filepath, GLubyte *pixels, GLint w, GLint h, int depth);

    GLint width() const { return mWidth; }
    GLint height() const { return mHeight; }
    GLint depth() const { return mDepth; }

    bool isLoaded() const { return mLoaded; }

    GLuint textureId() const { return tih->textureId(); }
	shared_ptr<TextureIdHolder> textureIdHolder() { return tih; }
};

#endif // GL_TEXTURE_H
