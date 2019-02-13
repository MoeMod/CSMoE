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
	class TextureNotFoundException : public std::runtime_error
	{
	public:
		TextureNotFoundException() : runtime_error("cannot load texture") {}
	};
public:
	explicit CTextureRef(const char *path, int flags = TF_NEAREST | TF_NOPICMIP | TF_NOMIPMAP | TF_CLAMP)
		: m_iInternalId(gRenderAPI.GL_LoadTexture(path, NULL, 0, flags))
	{
		if (!m_iInternalId)
			throw TextureNotFoundException();
	}

public:
	~CTextureRef() { gRenderAPI.GL_FreeTexture(m_iInternalId); }

public:
	void Bind(int tmu = 0) const noexcept
	{
		gRenderAPI.GL_SelectTexture(tmu);
		gRenderAPI.GL_Bind(tmu, m_iInternalId);
	}

	int GetParm(int parm = PARM_TEX_TYPE) const noexcept { return gRenderAPI.RenderGetParm(parm, m_iInternalId); }
	int w() const noexcept { return GetParm(PARM_TEX_WIDTH); }
	int h() const noexcept { return GetParm(PARM_TEX_HEIGHT); }
	int texnum() const noexcept { return m_iInternalId; }

private:
	const int m_iInternalId;
};

template<class...Args>
inline UniqueTexture R_LoadTextureUnique(Args&&...args)
{
	try
	{
		return std::unique_ptr<CTextureRef>(new CTextureRef(std::forward<Args>(args)...));
	}
	catch (const CTextureRef::TextureNotFoundException &e) {}
	return nullptr;
}
template<class...Args>
inline SharedTexture R_LoadTextureShared(Args&&...args)
{
	try
	{
		return std::make_shared<CTextureRef>(std::forward<Args>(args)...);
	}
	catch (const CTextureRef::TextureNotFoundException &e) {}
	return nullptr;
}

template<class P, class...Args>
inline bool R_InitTexture(P &p, Args&&...args)
{
	if (!p)
	{
		try
		{
			p = P(new CTextureRef(std::forward<Args>(args)...));
			return true;
		}
		catch (const CTextureRef::TextureNotFoundException &e) {}
	}
	return false;
}
