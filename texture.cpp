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

#include "common.h"

TextureIdHolder::TextureIdHolder()
{
	glGenTextures(1, &mTextureId);
}

TextureIdHolder::~TextureIdHolder()
{
	glDeleteTextures(1, &mTextureId);
}

Texture::Texture(GLenum target, GLenum wrapMode, GLenum filterMode, bool vFlip)//, bool sRGB)
    : mTarget(target),
      mWrapMode(wrapMode),
      mFilterMode(filterMode),
      mVFlip(vFlip),
      //msRGB(sRGB),
	  tih(make_shared<TextureIdHolder>()),
      mLoaded(false)
{
}

Texture::~Texture()
{
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

void Texture::createEmpty(int w, int h, int i)
{
    glBindTexture(GL_TEXTURE_2D, textureId());
	//glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, w, h);
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_HALF_FLOAT, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, textureId(), 0);

    mWidth = w;
    mHeight = h;
    mDepth = 1;

    set_params();
}

void Texture::resize(int w, int h, int i, TEnumResizeContent erc)
{
	glReadBuffer(GL_COLOR_ATTACHMENT0 + i);

	shared_ptr<TextureIdHolder> tih_new = make_shared<TextureIdHolder>();
	GLint i_active;
	glGetIntegerv(GL_ACTIVE_TEXTURE, &i_active);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tih_new->textureId());
	//glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, w, h);
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_HALF_FLOAT, nullptr);
	
	set_params();

	if(erc == ercFromBottomLeft)
	{
		glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, min(mWidth, w), min(mHeight, h));
	}
	else
	if(erc == ercFromCenter)
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
		glCopyTexSubImage2D(GL_TEXTURE_2D, 0, x_ofs, y_ofs, x, y, width, height);
	}

	glActiveTexture(i_active);
	tih = tih_new;
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, textureId(), 0);
	
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

    glBindTexture(mTarget, textureId());

    if (mTarget == GL_TEXTURE_CUBE_MAP) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + cubemapLayer, 0, format, w, h, 0, sourceFormat,
                     isFloat ? GL_FLOAT : GL_UNSIGNED_BYTE, pixels);
    } else if (mTarget == GL_TEXTURE_3D) {
        glTexImage3D(GL_TEXTURE_3D, 0, format, w, h, depth, 0, sourceFormat,
                     isFloat ? GL_FLOAT : GL_UNSIGNED_BYTE, pixels);
    } else {
        glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, sourceFormat,
                     isFloat ? GL_FLOAT : GL_UNSIGNED_BYTE, pixels);
    }

    glGenerateMipmap(mTarget);

    mWidth = w;
    mHeight = h;
    mDepth = depth;

    mLoaded = true;
}

void Texture::set_params(void)
{
    if (mWrapMode == GL_REPEAT) {
        glTexParameteri(mTarget, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(mTarget, GL_TEXTURE_WRAP_T, GL_REPEAT);
        if (mTarget == GL_TEXTURE_CUBE_MAP || mTarget == GL_TEXTURE_3D)
            glTexParameteri(mTarget, GL_TEXTURE_WRAP_R, GL_REPEAT);
    } else {
        glTexParameteri(mTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(mTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        if (mTarget == GL_TEXTURE_CUBE_MAP || mTarget == GL_TEXTURE_3D)
            glTexParameteri(mTarget, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }

    if (mFilterMode == GL_NEAREST) {
        glTexParameteri(mTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(mTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    } else if (mFilterMode == GL_MIPMAP) {
        glTexParameteri(mTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(mTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glGenerateMipmap(mTarget);
    } else {
        glTexParameteri(mTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(mTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }	
}

void Texture::bindToChannel(int channel)
{
    glActiveTexture(GL_TEXTURE0 + channel);
    glBindTexture(mTarget, textureId());
}

void Texture::release()
{
    glBindTexture(mTarget, 0);
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
