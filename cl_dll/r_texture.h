#pragma once

#include "cl_dll.h"

#include <memory>

class CTextureRef; // forward decl
using SharedTexture = std::shared_ptr<CTextureRef>;
using UniqueTexture = std::unique_ptr<CTextureRef>;
using WeakTexture = std::weak_ptr<CTextureRef>;

class CTextureRef : public std::enable_shared_from_this<CTextureRef>
{
public:
	explicit CTextureRef(const char *path, int flags)
		: m_iInternalId(gRenderAPI.GL_LoadTexture(path, NULL, 0, flags)) {}

public:
	~CTextureRef() { gRenderAPI.GL_FreeTexture(m_iInternalId); }
	
public:
	bool IsValid() const { return m_iInternalId != 0; }
	operator bool() const { return IsValid(); }

	void Bind(int tmu = 0) const
	{
		gRenderAPI.GL_SelectTexture(0);
		gRenderAPI.GL_Bind(0, m_iInternalId);
	}

	int GetParm(int parm = PARM_TEX_TYPE) const { return gRenderAPI.RenderGetParm(parm, m_iInternalId); }
	int w() const {return GetParm(PARM_TEX_WIDTH);}
	int h() const { return GetParm(PARM_TEX_HEIGHT); }
	int texnum() const { return m_iInternalId; }

protected:
	int m_iInternalId;
};

inline UniqueTexture R_LoadTextureUnique(const char *path, int flags = TF_NEAREST | TF_NOPICMIP | TF_NOMIPMAP | TF_CLAMP)
{
	// C++11 doesn't have make_unique
	return std::unique_ptr<CTextureRef>(new CTextureRef(path, flags));
}

inline SharedTexture R_LoadTextureShared(const char *path, int flags = TF_NEAREST | TF_NOPICMIP | TF_NOMIPMAP | TF_CLAMP)
{
	return std::make_shared<CTextureRef>(path, flags);
}