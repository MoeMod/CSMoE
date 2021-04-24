/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//
// cl_util.h
//
#pragma once
#include "cvardef.h"
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

extern cvar_t *hud_textmode;

#ifdef _MSC_VER
#pragma warning(disable : 4244) // 'argument': conversion from 'float' to 'int', possible loss of data
#pragma warning(disable : 4101) // unreferenced local variable
#endif

// Macros to hook function calls into the HUD object
#define HOOK_MESSAGE(x) gEngfuncs.pfnHookUserMsg(#x, __MsgFunc_##x );

#define DECLARE_MESSAGE(y, x) int __MsgFunc_##x(const char *pszName, int iSize, void *pbuf) \
{ \
	return gHUD.y.MsgFunc_##x(pszName, iSize, pbuf ); \
	}

#define HOOK_COMMAND(x, y) gEngfuncs.pfnAddCommand( x, __CmdFunc_##y );
#define DECLARE_COMMAND(y, x) void __CmdFunc_##x( void ) \
{ \
	gHUD.y.UserCmd_##x( ); \
	}

inline float CVAR_GET_FLOAT( const char *x ) {	return gEngfuncs.pfnGetCvarFloat( x ); }
inline char* CVAR_GET_STRING( const char *x ) {	return gEngfuncs.pfnGetCvarString( x ); }
inline struct cvar_s *CVAR_CREATE( const char *cv, const char *val, const int flags ) {	return gEngfuncs.pfnRegisterVariable( cv, val, flags ); }

inline HSPRITE SPR_Load( const char *szPicName ) { return (*gEngfuncs.pfnSPR_Load)(szPicName);}
inline void SPR_Set( HSPRITE hPic, int r, int g, int b ) { return (*gEngfuncs.pfnSPR_Set)(hPic, r, g, b);}
inline int SPR_Frames( HSPRITE hPic ) { return (*gEngfuncs.pfnSPR_Frames)(hPic); }
inline client_sprite_t *SPR_GetList( const char *psz, int *piCount ) { return (*gEngfuncs.pfnSPR_GetList)(psz, piCount); }

// SPR_Draw  draws a the current sprite as solid
inline void SPR_Draw( int frame, int x, int y, const wrect_t *prc ) { return (*gEngfuncs.pfnSPR_Draw)(frame, x, y, prc); }
// SPR_DrawHoles  draws the current sprites,  with color index255 not drawn (transparent)
inline void SPR_DrawHoles( int frame, int x, int y, const wrect_t *prc ) { return (*gEngfuncs.pfnSPR_DrawHoles)(frame, x, y, prc); }
// SPR_DrawAdditive  adds the sprites RGB values to the background  (additive transulency)
inline void SPR_DrawAdditive( int frame, int x, int y, const wrect_t *prc ) { return (*gEngfuncs.pfnSPR_DrawAdditive)(frame, x, y, prc); }
// SPR_EnableScissor  sets a clipping rect for HUD sprites.  (0,0) is the top-left hand corner of the screen.
inline void SPR_EnableScissor( int x, int y, int width, int height ) { return (*gEngfuncs.pfnSPR_EnableScissor)(x, y, width, height); }
// SPR_DisableScissor  disables the clipping rect
inline void SPR_DisableScissor(  ) { return (*gEngfuncs.pfnSPR_DisableScissor)(); }
//
inline void	FillRGBA( int x, int y, int width, int height, int r, int g, int b, int a ) { return (*gEngfuncs.pfnFillRGBA)(x, y, width, height, r, g, b, a); }
inline void	FillRGBABlend( int x, int y, int width, int height, int r, int g, int b, int a ) { return (*gEngfuncs.pfnFillRGBABlend)(x, y, width, height, r, g, b, a); }

// ScreenHeight returns the height of the screen, in pixels
// ScreenWidth returns the width of the screen, in pixels
namespace detail {
struct ScreenHeightGetterProxy { operator int() const { return gHUD.m_scrinfo.iHeight; } };
struct ScreenWidthGetterProxy { operator int() const { return gHUD.m_scrinfo.iWidth; } };
struct TrueHeightGetterProxy { operator int() const { return gHUD.m_truescrinfo.iHeight; } };
struct TrueWidthGetterProxy { operator int() const { return gHUD.m_truescrinfo.iWidth; } };
}
constexpr detail::ScreenHeightGetterProxy ScreenHeight;
constexpr detail::ScreenWidthGetterProxy ScreenWidth;
constexpr detail::TrueHeightGetterProxy TrueHeight;
constexpr detail::TrueWidthGetterProxy TrueWidth;

// Use this to set any co-ords in 640x480 space
inline int XRES(float x) { return static_cast<int>(x * (static_cast<float>(ScreenWidth) / 640.0f) + 0.5f);  }
inline int YRES(float y) { return static_cast<int>(y * (static_cast<float>(ScreenHeight) / 480.0f) + 0.5f);  }

// use this to project world coordinates to screen coordinates
inline float XPROJECT(float x) { return (1.0f + x) * ScreenWidth * 0.5f; }
inline float YPROJECT(float y) { return (1.0f - y) * ScreenHeight * 0.5f; }

inline int GetScreenInfo( SCREENINFO *pscrinfo ) { return gEngfuncs.pfnGetScreenInfo(pscrinfo); }
inline int ServerCmd( const char *szCmdString ) { return gEngfuncs.pfnServerCmd(szCmdString); }
inline int ClientCmd( const char *szCmdString ) { return gEngfuncs.pfnClientCmd(szCmdString); }
inline void SetCrosshair( HSPRITE hspr, wrect_t rc, int r, int g, int b ) { return gEngfuncs.pfnSetCrosshair(hspr, rc, r, g, b); }
inline int Com_RandomLong( int lLow, int lHigh ) { return gEngfuncs.pfnRandomLong(lLow, lHigh); }
inline float Com_RandomFloat( float flLow, float flHigh ) { return gEngfuncs.pfnRandomFloat(flLow, flHigh); }

extern float color[3]; // hud.cpp

// Gets the height & width of a sprite,  at the specified frame
inline int SPR_Height( HSPRITE x, int f ) { return gEngfuncs.pfnSPR_Height(x, f); }
inline int SPR_Width( HSPRITE x, int f ) { return gEngfuncs.pfnSPR_Width(x, f); }
inline client_textmessage_t *TextMessageGet( const char *pName ) { return gEngfuncs.pfnTextMessageGet( pName ); }
inline void ConsolePrint( const char *string ) { gEngfuncs.pfnConsolePrint( string );}
inline void CenterPrint( const char *string ) { gEngfuncs.pfnCenterPrint( string ); }

// returns the players name of entity no.
inline void GetPlayerInfo( int ent_num, hud_player_info_t *pinfo ) { return gEngfuncs.pfnGetPlayerInfo(ent_num, pinfo); }

// sound functions
inline void PlaySound( const char *szSound, float vol ) { gEngfuncs.pfnPlaySoundByName( szSound, vol ); }
inline void PlaySound( int iSound, float vol ) { gEngfuncs.pfnPlaySoundByIndex( iSound, vol ); }

#include "minmax.h"

#ifdef VectorSubtract
#undef VectorSubtract
#endif
#ifdef VectorAdd
#undef VectorAdd
#endif
#ifdef VectorCopy
#undef VectorCopy
#endif
#ifdef VectorClear
#undef VectorClear
#endif

template<class VectorTypeA, class VectorTypeB> auto DotProduct(const VectorTypeA &x, const VectorTypeB & y) -> decltype((x)[0] * (y)[0] + (x)[1] * (y)[1] + (x)[2] * (y)[2]) { return ((x)[0] * (y)[0] + (x)[1] * (y)[1] + (x)[2] * (y)[2]); }
template<class VectorTypeA, class VectorTypeB, class VectorTypeC> void VectorSubtract(const VectorTypeA & a, const VectorTypeB & b, VectorTypeC &c) { (c)[0] = (a)[0] - (b)[0]; (c)[1] = (a)[1] - (b)[1]; (c)[2] = (a)[2] - (b)[2]; }
template<class VectorTypeA, class VectorTypeB, class VectorTypeC> void VectorAdd(const VectorTypeA & a, const VectorTypeB & b, VectorTypeC &c) { (c)[0] = (a)[0] + (b)[0]; (c)[1] = (a)[1] + (b)[1]; (c)[2] = (a)[2] + (b)[2]; }
template<class VectorTypeA, class VectorTypeB> void VectorCopy(const VectorTypeA & a, VectorTypeB & b) { (b)[0] = (a)[0]; (b)[1] = (a)[1]; (b)[2] = (a)[2]; }
template<class VectorTypeA> void VectorClear(VectorTypeA &a) { a[0] = 0.0; a[1] = 0.0; a[2] = 0.0; }
template<class VectorTypeA> auto VectorLength(const VectorTypeA &a) -> decltype(sqrt(DotProduct(a, a))) { return sqrt(DotProduct(a, a)); }
template<class VectorTypeA, class ScaleType, class VectorTypeB, class VectorTypeC> void VectorMA(const VectorTypeA &a, ScaleType scale, const VectorTypeB &b, VectorTypeC &c) { ((c)[0] = (a)[0] + (scale) * (b)[0], (c)[1] = (a)[1] + (scale) * (b)[1], (c)[2] = (a)[2] + (scale) * (b)[2]); }
template<class VectorTypeA, class ScaleType, class VectorTypeB> void VectorScale(const VectorTypeA &in, ScaleType scale, VectorTypeB &out) { ((out)[0] = (in)[0] * (scale), (out)[1] = (in)[1] * (scale), (out)[2] = (in)[2] * (scale)); }
template<class VectorTypeA> void VectorInverse(VectorTypeA &x) { ((x)[0] = -(x)[0], (x)[1] = -(x)[1], (x)[2] = -(x)[2]); }
template<class VectorTypeA> void AngleVectors( const VectorTypeA &vecAngles, float *forward, float *right, float *up ) { return gEngfuncs.pfnAngleVectors(vecAngles, forward, right, up); }

namespace cl{
	// const vec3_t ==untypedef=> float (const [3]) ==decay=> float *		NOT const float * !!!
	float VectorNormalize(vec_t v[3]); // pm_math.h
	//extern vec3_t vec3_origin;
}

#ifdef MSC_VER
// disable 'possible loss of data converting float to int' warning message
#pragma warning( disable: 4244 )
// disable 'truncation from 'const double' to 'float' warning message
#pragma warning( disable: 4305 )
#endif

float *GetClientColor( int clientIndex );
inline HSPRITE LoadSprite(const char *pszName)
{
	char sz[256];
	snprintf(sz, 256, pszName, 640);

	return SPR_Load(sz);
}

extern vec3_t g_ColorRed, g_ColorBlue, g_ColorYellow, g_ColorGrey;

inline void GetTeamColor( int &r, int &g, int &b, int teamIndex )
{
	r = 255;
	g = 255;
	b = 255;
	switch( teamIndex )
	{
	case TEAM_TERRORIST:
		r *= g_ColorRed[0];
		g *= g_ColorRed[1];
		b *= g_ColorRed[2];
		break;
	case TEAM_CT:
		r *= g_ColorBlue[0];
		g *= g_ColorBlue[1];
		b *= g_ColorBlue[2];
		break;
	case TEAM_SPECTATOR:
	case TEAM_UNASSIGNED:
		r *= g_ColorYellow[0];
		g *= g_ColorYellow[1];
		b *= g_ColorYellow[2];
		break;
	default:
		r *= g_ColorGrey[0];
		g *= g_ColorGrey[1];
		b *= g_ColorGrey[2];
		break;
	}
}

template<class MinType, class NumType, class MaxType>
constexpr auto bound(MinType min, NumType num, MaxType max) -> typename std::common_type<MinType, NumType, MaxType>::type
{
	return ((num) >= (min) ? ((num) < (max) ? (num) : (max)) : (min));
}

