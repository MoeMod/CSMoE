#pragma once
/*
	r_texture.h

	RAII wrap for GL texture in gRenderAPI
*/
#include "render_api.h"
#if defined(XASH_IMGUI) && !defined(XASH_VGUI2)
#include "imgui.h"
#else
#include "triangleapi.h"
#endif

#include <memory>

#if defined(XASH_IMGUI) && !defined(XASH_VGUI2)
extern float g_ImGUI_DPI;
extern void ImGui_Surface_SetRenderMode( int mode );
#else
// extern cvar_t *hud_scale;
#endif

namespace cl {

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
	void Draw2DQuadScaled( float x1, float y1, float x2, float y2, float s1 = 0, float t1 = 0, float s2 = 1, float t2 = 1, byte r = 255, byte g = 255, byte b = 255, byte a = 255 ) const noexcept
	{
#if defined(XASH_IMGUI) && !defined(XASH_VGUI2)
		Draw2DQuad(x1 * g_ImGUI_DPI, y1 * g_ImGUI_DPI, x2 * g_ImGUI_DPI, y2 * g_ImGUI_DPI, s1, t1, s2, t2, r, g, b, a);
#else
		float flScale = gEngfuncs.pfnGetCvarFloat("hud_scale");
        Draw2DQuad(x1 * flScale, y1 * flScale, x2 * flScale, y2 * flScale, s1, t1, s2, t2, r, g, b, a);
#endif
	}

	void Draw2DQuad(float x1, float y1, float x2, float y2, float s1 = 0, float t1 = 0, float s2 = 1, float t2 = 1, byte r = 255, byte g = 255, byte b = 255, byte a = 255) const noexcept
	{
#if defined(XASH_IMGUI) && !defined(XASH_VGUI2)
		ImDrawList* drawlist = ImGui::GetBackgroundDrawList();
		ImGui_Surface_SetRenderMode(kRenderTransTexture);
		drawlist->AddImage((ImTextureID)m_iInternalId, ImVec2(x1, y1), ImVec2(x2, y2), ImVec2(s1, t1), ImVec2(s2, t2), ImColor(r, g, b, a));
#else
		gEngfuncs.pTriAPI->RenderMode(kRenderTransTexture);
		gEngfuncs.pTriAPI->Color4ub(r, g, b, a);
		gRenderAPI.GL_Bind(0, m_iInternalId);
		gEngfuncs.pTriAPI->Begin( TRI_QUADS );
		gEngfuncs.pTriAPI->TexCoord2f( s1, t1 );
		gEngfuncs.pTriAPI->Vertex3f( x1, y1, 0 );
		gEngfuncs.pTriAPI->TexCoord2f( s1, t2 );
		gEngfuncs.pTriAPI->Vertex3f( x1, y2, 0 );
		gEngfuncs.pTriAPI->TexCoord2f( s2, t2 );
		gEngfuncs.pTriAPI->Vertex3f( x2, y2, 0 );
		gEngfuncs.pTriAPI->TexCoord2f( s2, t1 );
		gEngfuncs.pTriAPI->Vertex3f( x2, y1, 0 );
		gEngfuncs.pTriAPI->End( );
#endif
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

}