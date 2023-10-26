#include "common.h"
#include "texture.h"
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#include "stb_image_write.h"
#pragma GCC diagnostic push

#include "TGles2Fns.h"

Texture::Texture(GLenum target, GLenum wrapMode, GLenum filterMode, bool vFlip)//, bool sRGB)
    : mTarget(target),
      mWrapMode(wrapMode),
      mFilterMode(filterMode),
      mVFlip(vFlip),
      //msRGB(sRGB),
      mTextureId(0),
      mLoaded(false)
{
    TGles2Fns::glGenTextures(1, &mTextureId);
}

Texture::~Texture()
{
    TGles2Fns::glDeleteTextures(1, &mTextureId);
}

void Texture::loadFromFile(const char*filename)
{
    stbi_set_flip_vertically_on_load(mVFlip);
    int w, h, n;
    GLubyte *pixels = stbi_load(filename, &w, &h, &n, STBI_rgb);
    if (pixels == nullptr)
        return;
	
    if (mTarget == GL_TEXTURE_CUBE_MAP) {
        for (int i = 0; i < 6; i++) {
            loadPixels(&pixels[i * w * w * n], w, h, 1, n, false, i);
        }
    } else {
        loadPixels(pixels, w, h, 1, n, false, 0);
    }
	
	stbi_image_free(pixels);
    pixels = nullptr;
}

void Texture::loadFromePixels(GLubyte *pixels, int w, int h, int depth, int channels, bool isFloat)
{
    loadPixels(pixels, w, h, depth, channels, isFloat, 0);
}

void Texture::createEmpty(int w, int h)
{
    TGles2Fns::glBindTexture(GL_TEXTURE_2D, mTextureId);
    TGles2Fns::glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, w, h);
	//TGles2Fns::glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    TGles2Fns::glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextureId, 0);

    mWidth = w;
    mHeight = h;
    mDepth = 1;

	set_params();
}

void Texture::resize(int w, int h, TEnumResizeContent erc)
{
	TGles2Fns::glReadBuffer(GL_COLOR_ATTACHMENT0);

	GLuint mNewTextureId;
	TGles2Fns::glGenTextures(1, &mNewTextureId);
	GLint i_active;
	TGles2Fns::glGetIntegerv(GL_ACTIVE_TEXTURE, &i_active);
	TGles2Fns::glActiveTexture(GL_TEXTURE0);
    TGles2Fns::glBindTexture(GL_TEXTURE_2D, mNewTextureId);
    //TGles2Fns::glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	TGles2Fns::glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, w, h);
	
	set_params();

	switch (erc)
	{
		case ercNone:
			break;

		case ercFromBottomLeft:
		{
			TGles2Fns::glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, min(mWidth, w), min(mHeight, h));
			break;
		}
		
		case ercFromCenter:
		{	
			int x_ofs, y_ofs, x, y, width, height;
			if(w > mWidth)
			{
				width = mWidth;
				x_ofs = (w - mWidth) >> 1;
				x = 0;
			}
			else
			{
				width = w;
				x_ofs = 0;
				x = (mWidth - w) >> 1;
			}

			if(h > mHeight)
			{
				height = mHeight;
				y_ofs = (h - mHeight) >> 1;
				y = 0;
			}
			else
			{
				height = h;
				y_ofs = 0;
				y = (mHeight - h) >> 1;
			}
			TGles2Fns::glCopyTexSubImage2D(GL_TEXTURE_2D, 0, x_ofs, y_ofs, x, y, width, height);
			
			break;
		}
	}

	TGles2Fns::glActiveTexture(i_active);
	TGles2Fns::glDeleteTextures(1, &mTextureId);
	
	mTextureId = mNewTextureId;
    TGles2Fns::glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextureId, 0);
	
	mWidth = w;
	mHeight = h;
}

void Texture::loadPixels(GLubyte *pixels, int w, int h, int depth, int channels, bool isFloat, int cubemapLayer)
{
    GLenum format = GL_RGBA;
    GLenum sourceFormat = GL_RGBA;

    if (channels == 1) {
        format = isFloat ? GL_R16F : GL_R8;
        sourceFormat = GL_RGB;
    } else if (channels == 2) {
        format = isFloat ? GL_RG16F : GL_RG8;
        sourceFormat = GL_RG;
    } else if (channels == 3) {
        format = isFloat ? GL_RGB16F : GL_RGB8;
        sourceFormat = GL_RGB;
    } else if (channels == 4) {
        format = isFloat ? GL_RGBA16F : GL_RGBA8;
        sourceFormat = GL_RGBA;
    }

    TGles2Fns::glBindTexture(mTarget, mTextureId);

    if (mTarget == GL_TEXTURE_CUBE_MAP) {
        TGles2Fns::glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + cubemapLayer, 0, format, w, h, 0, sourceFormat,
                     isFloat ? GL_FLOAT : GL_UNSIGNED_BYTE, pixels);
    } else if (mTarget == GL_TEXTURE_3D) {
        TGles2Fns::glTexImage3D(GL_TEXTURE_3D, 0, format, w, h, depth, 0, sourceFormat,
                     isFloat ? GL_FLOAT : GL_UNSIGNED_BYTE, pixels);
    } else {
        TGles2Fns::glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, sourceFormat,
                     isFloat ? GL_FLOAT : GL_UNSIGNED_BYTE, pixels);
    }

    TGles2Fns::glGenerateMipmap(mTarget);

    mWidth = w;
    mHeight = h;
    mDepth = depth;

    mLoaded = true;
}

void Texture::set_params(void)
{
    if (mWrapMode == GL_REPEAT) {
        TGles2Fns::glTexParameteri(mTarget, GL_TEXTURE_WRAP_S, GL_REPEAT);
        TGles2Fns::glTexParameteri(mTarget, GL_TEXTURE_WRAP_T, GL_REPEAT);
        if (mTarget == GL_TEXTURE_CUBE_MAP || mTarget == GL_TEXTURE_3D)
            TGles2Fns::glTexParameteri(mTarget, GL_TEXTURE_WRAP_R, GL_REPEAT);
    } else {
        TGles2Fns::glTexParameteri(mTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        TGles2Fns::glTexParameteri(mTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        if (mTarget == GL_TEXTURE_CUBE_MAP || mTarget == GL_TEXTURE_3D)
            TGles2Fns::glTexParameteri(mTarget, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }

    if (mFilterMode == GL_NEAREST) {
        TGles2Fns::glTexParameteri(mTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        TGles2Fns::glTexParameteri(mTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    } else if (mFilterMode == GL_MIPMAP) {
        TGles2Fns::glTexParameteri(mTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        TGles2Fns::glTexParameteri(mTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    } else {
        TGles2Fns::glTexParameteri(mTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        TGles2Fns::glTexParameteri(mTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }	
}

void Texture::bindToChannel(int channel)
{
    TGles2Fns::glActiveTexture(GL_TEXTURE0 + channel);
    TGles2Fns::glBindTexture(mTarget, mTextureId);

    /*if (mWrapMode == GL_REPEAT) {
        TGles2Fns::glTexParameteri(mTarget, GL_TEXTURE_WRAP_S, GL_REPEAT);
        TGles2Fns::glTexParameteri(mTarget, GL_TEXTURE_WRAP_T, GL_REPEAT);
        if (mTarget == GL_TEXTURE_CUBE_MAP || mTarget == GL_TEXTURE_3D)
            TGles2Fns::glTexParameteri(mTarget, GL_TEXTURE_WRAP_R, GL_REPEAT);
    } else {
        TGles2Fns::glTexParameteri(mTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        TGles2Fns::glTexParameteri(mTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        if (mTarget == GL_TEXTURE_CUBE_MAP || mTarget == GL_TEXTURE_3D)
            TGles2Fns::glTexParameteri(mTarget, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }

    if (mFilterMode == GL_NEAREST) {
        TGles2Fns::glTexParameteri(mTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        TGles2Fns::glTexParameteri(mTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    } else if (mFilterMode == GL_MIPMAP) {
        TGles2Fns::glTexParameteri(mTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        TGles2Fns::glTexParameteri(mTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    } else {
        TGles2Fns::glTexParameteri(mTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        TGles2Fns::glTexParameteri(mTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }*/
}

void Texture::release()
{
    TGles2Fns::glBindTexture(mTarget, 0);
}

bool Texture::savePixelsToFile(const char *filepath, GLubyte *pixels, GLint w, GLint h, int depth)
{
    stbi_uc temp;

    for (int row = 0; row < (h >> 1); row++) {
        for (int col = 0; col < w; col++) {
            for (int z = 0; z < depth; z++) {
                int idx1 = (row * w + col) * depth + z;
                int idx2 = ((h - row - 1) * w + col) * depth + z;
                temp = pixels[idx1];
                pixels[idx1] = pixels[idx2];
                pixels[idx2] = temp;
            }
        }
    }

    return stbi_write_png(filepath, w, h, depth, pixels, w * depth);
}
