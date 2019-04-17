#pragma once
/*
	r_texture.h

	RAII wrap for GL texture in gRenderAPI
*/
#include "render_api.h"

#include <memory>

class CTextureRef; // forward decl
using SharedTexture = std::shared_ptr<CTextureRef>;
using UniqueTexture = std::unique_ptr<CTextureRef>;
using WeakTexture = std::weak_ptr<CTextureRef>;

class CTextureRef
{
public:
	explicit CTextureRef(const char *path, int flags = TF_NEAREST | TF_NOPICMIP | TF_NOMIPMAP | TF_CLAMP)
		: m_iInternalId(g_iXash ? gRenderAPI.GL_LoadTexture(path, NULL, 0, flags) : 0) {}
	CTextureRef(const CTextureRef&) = delete;
	CTextureRef(CTextureRef &&rhs) : m_iInternalId(0) { std::swap(m_iInternalId, rhs.m_iInternalId); }
	CTextureRef &operator=(const CTextureRef&) const = delete;
	CTextureRef &operator=(CTextureRef &&rhs) { std::swap(m_iInternalId, rhs.m_iInternalId); return *this; }

public:
	~CTextureRef() 
	{ 
		if(g_iXash) 
			gRenderAPI.GL_FreeTexture(m_iInternalId); 
	}

public:
	void Bind(int tmu = 0) const noexcept
	{
		if (g_iXash)
		{
			gRenderAPI.GL_SelectTexture(tmu);
			gRenderAPI.GL_Bind(tmu, m_iInternalId);
		}
	}

	int GetParm(int parm = PARM_TEX_TYPE) const noexcept 
	{ 
		if(g_iXash)
			return gRenderAPI.RenderGetParm(parm, m_iInternalId); 
		return 0;
	}
	int w() const noexcept { return GetParm(PARM_TEX_WIDTH); }
	int h() const noexcept { return GetParm(PARM_TEX_HEIGHT); }
	int texnum() const noexcept { return m_iInternalId; }
	bool valid() const noexcept { return texnum() > 0; }

private:
	unsigned int m_iInternalId;
};

template<class...Args>
inline UniqueTexture R_LoadTextureUnique(Args&&...args)
{
	CTextureRef tex(std::forward<Args>(args)...);
	return tex.valid() ? std::unique_ptr<CTextureRef>(new CTextureRef(std::move(tex))) : nullptr;
}

template<class...Args>
inline SharedTexture R_LoadTextureShared(Args&&...args)
{
	CTextureRef tex(std::forward<Args>(args)...);
	return tex.valid() ? std::make_shared<CTextureRef>(std::move(tex)) : nullptr;
}

template<class P, class...Args>
inline bool R_InitTexture(P &p, Args&&...args)
{
	if (!p)
	{
		CTextureRef tex(std::forward<Args>(args)...);
		if (tex.valid())
			return static_cast<bool>(p = P(new CTextureRef(std::move(tex))));
	}
	return false;
}
