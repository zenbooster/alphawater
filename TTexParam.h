#pragma once
#include <unordered_set>

struct TTexParam
{
	GLenum wrapMode;
	GLenum filterMode;
	
	bool operator==(const TTexParam& o) const
	{
		return wrapMode == o.wrapMode && filterMode == o.filterMode;
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