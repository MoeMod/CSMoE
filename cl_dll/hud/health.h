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
#pragma once
constexpr int DMG_IMAGE_LIFE = 2;	// seconds that image is up
constexpr int DMG_IMAGE_POISON = 0;
constexpr int DMG_IMAGE_ACID = 1;
constexpr int DMG_IMAGE_COLD = 2;
constexpr int DMG_IMAGE_DROWN = 3;
constexpr int DMG_IMAGE_BURN = 4;
constexpr int DMG_IMAGE_NERVE = 5;
constexpr int DMG_IMAGE_RAD = 6;
constexpr int DMG_IMAGE_SHOCK = 7;
//tf defines
constexpr int DMG_IMAGE_CALTROP = 8;
constexpr int DMG_IMAGE_TRANQ = 9;
constexpr int DMG_IMAGE_CONCUSS = 10;
constexpr int DMG_IMAGE_HALLUC = 11;
constexpr size_t NUM_DMG_TYPES = 12;
// instant damage

namespace cl {

struct DAMAGE_IMAGE
{
	float fExpire;
	float fBaseline;
	int	x, y;
};

//
//-----------------------------------------------------
//
class CHudHealth: public CHudBase
{
public:
	virtual int Init( void );
	virtual int VidInit( void );
	virtual int Draw(float fTime);
	virtual void Reset( void );
	int DrawBar(int x, int y, int width, int height, float f, int& r, int& g, int& b, int& a);
	int MsgFunc_Health(const char *pszName,  int iSize, void *pbuf);
	int MsgFunc_Damage(const char *pszName,  int iSize, void *pbuf);
	int MsgFunc_ScoreAttrib(const char *pszName,  int iSize, void *pbuf);
	int MsgFunc_ClCorpse(const char *pszName,  int iSize, void *pbuf);

	int m_iHealth;
	int m_iMaxHealth;
	int m_HUD_dmg_bio;
	int m_HUD_cross;
	int m_NEWHUD_cross;
	//float m_fAttackFront, m_fAttackRear, m_fAttackLeft, m_fAttackRight;
	float m_fAttack[4];
	void GetPainColor(int &r, int &g, int &b , int &a);
	float m_fFade;
private:
	void DrawNewHudHealth(float fTime);
	void DrawPain( float fTime );
	void DrawDamage( float fTime );
	void DrawHealthBar( float flTime );
	void CalcDamageDirection( Vector vecFrom );
	void UpdateTiles( float fTime, long bits );

	HSPRITE m_hSprite;
	HSPRITE m_hDamage;
	Vector2D m_vAttackPos[4];
	DAMAGE_IMAGE m_dmg[NUM_DMG_TYPES];
	float m_flTimeFlash;
	int	m_bitsDamage;
	cvar_t *cl_radartype;
	UniqueTexture m_pTexture_Black;
};
}