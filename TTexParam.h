#pragma once
#include <unordered_set>

struct TTexParam
{
	GLenum wrapMode;
	GLenum filterMode;
	
	TTexParam(GLenum w = GL_CLAMP, GLenum f = GL_LINEAR):
		wrapMode(w),
		filterMode(f)
	{}
	
	bool operator==(const TTexParam& o) const
	{
		return wrapMode == o.wrapMode && filterMode == o.filterMode;
	}

	operator string() const
	{
		static unordered_map<GLenum, string> w2c = {
			{GL_CLAMP, "c"}, 
			{GL_REPEAT, "r"}
		};
		static unordered_map<GLenum, string> f2c = {
			{GL_LINEAR, "l"}, 
			{GL_NEAREST, "n"},
			{GL_MIPMAP, "m"}
		};

		return tostringstream() << "TTexParam(w=" << w2c[wrapMode] << ", f=" << f2c[filterMode] << ")";
	}
};

class TTexParamHashFunction
{
	public:
		size_t operator()(const TTexParam& o) const
		{
			return (hash<GLenum>()(o.wrapMode)) ^ (hash<GLenum>()(o.filterMode));
		}
};

typedef unordered_set<TTexParam, TTexParamHashFunction> TTexParams;