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
// Health.cpp
//
// implementation of CHudHealth class
//

#include "stdio.h"
#include "stdlib.h"
#include "math.h"

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include <string.h>
#include "eventscripts.h"

#include "draw_util.h"
#include "const/const_client.h"

namespace cl {

DECLARE_MESSAGE(m_Health, Health )
DECLARE_MESSAGE(m_Health, Damage )
DECLARE_MESSAGE(m_Health, ScoreAttrib )
DECLARE_MESSAGE(m_Health, ClCorpse )

constexpr auto PAIN_NAME = "sprites/%d_pain.spr";
constexpr auto DAMAGE_NAME = "sprites/%d_dmg.spr";
constexpr auto EPSILON = 0.4f;

int giDmgHeight, giDmgWidth;

float g_LocationColor[3];

int giDmgFlags[NUM_DMG_TYPES] =
{
	DMG_POISON,
	DMG_ACID,
	DMG_FREEZE|DMG_SLOWFREEZE,
	DMG_DROWN,
	DMG_BURN|DMG_SLOWBURN,
	DMG_NERVEGAS,
	DMG_RADIATION,
	DMG_SHOCK,
	DMG_CALTROP,
	DMG_TRANQ,
	DMG_CONCUSS,
	DMG_HALLUC
};

enum
{
	ATK_FRONT = 0,
	ATK_RIGHT,
	ATK_REAR,
	ATK_LEFT
};

int CHudHealth::Init(void)
{
	HOOK_MESSAGE(Health);
	HOOK_MESSAGE(Damage);
	HOOK_MESSAGE(ScoreAttrib);
	HOOK_MESSAGE(ClCorpse);

	m_iHealth = 100;
	m_fFade = 0;
	m_iFlags = 0;
	m_bitsDamage = 0;
	giDmgHeight = 0;
	giDmgWidth = 0;

	for( int i = 0; i < 4; i++ )
		m_fAttack[i] = 0;

	memset(m_dmg, 0, sizeof(DAMAGE_IMAGE) * NUM_DMG_TYPES);

	CVAR_CREATE("cl_corpsestay", "600", FCVAR_ARCHIVE);
	gHUD.AddHudElem(this);
	return 1;
}

void CHudHealth::Reset( void )
{
	// make sure the pain compass is cleared when the player respawns
	for( int i = 0; i < 4; i++ )
		m_fAttack[i] = 0;


	// force all the flashing damage icons to expire
	m_bitsDamage = 0;
	for ( size_t i = 0; i < NUM_DMG_TYPES; i++ )
	{
		m_dmg[i].fExpire = 0;
	}
}

int CHudHealth::VidInit(void)
{
	m_hSprite = LoadSprite(PAIN_NAME);

	m_vAttackPos[ATK_FRONT].x = ScreenWidth  / 2 - SPR_Width ( m_hSprite, 0 ) / 2;
	m_vAttackPos[ATK_FRONT].y = ScreenHeight / 2 - SPR_Height( m_hSprite, 0 ) * 3;

	m_vAttackPos[ATK_RIGHT].x = ScreenWidth  / 2 + SPR_Width ( m_hSprite, 1 ) * 2;
	m_vAttackPos[ATK_RIGHT].y = ScreenHeight / 2 - SPR_Height( m_hSprite, 1 ) / 2;

	m_vAttackPos[ATK_REAR ].x = ScreenWidth  / 2 - SPR_Width ( m_hSprite, 2 ) / 2;
	m_vAttackPos[ATK_REAR ].y = ScreenHeight / 2 + SPR_Height( m_hSprite, 2 ) * 2;

	m_vAttackPos[ATK_LEFT ].x = ScreenWidth  / 2 - SPR_Width ( m_hSprite, 3 ) * 3;
	m_vAttackPos[ATK_LEFT ].y = ScreenHeight / 2 - SPR_Height( m_hSprite, 3 ) / 2;


	m_HUD_dmg_bio = gHUD.GetSpriteIndex( "dmg_bio" ) + 1;
	m_HUD_cross = gHUD.GetSpriteIndex( "cross" );
	m_NEWHUD_cross = gHUD.GetSpriteIndex("cross_new");
	R_InitTexture(m_pTexture_Black, "resource/hud/csgo/blackleft");
	giDmgHeight = gHUD.GetSpriteRect(m_HUD_dmg_bio).right - gHUD.GetSpriteRect(m_HUD_dmg_bio).left;
	giDmgWidth = gHUD.GetSpriteRect(m_HUD_dmg_bio).bottom - gHUD.GetSpriteRect(m_HUD_dmg_bio).top;

	return 1;
}

int CHudHealth:: MsgFunc_Health(const char *pszName,  int iSize, void *pbuf )
{
	// TODO: update local health data
	BufferReader reader( pszName, pbuf, iSize );
	//int x = reader.ReadByte();
	int x = m_iHealth;

	if (iSize == 4)
	{
		x = reader.ReadShort();
		m_iMaxHealth = reader.ReadShort();
	}
	else
	{
		x = reader.ReadByte();
	}

	m_iFlags |= HUD_DRAW;

	// Only update the fade if we've changed health
	if (x != m_iHealth)
	{
		m_fFade = FADE_TIME;
		m_iHealth = x;
	}

	return 1;
}


int CHudHealth:: MsgFunc_Damage(const char *pszName,  int iSize, void *pbuf )
{
	BufferReader reader( pszName, pbuf, iSize );

	int armor = reader.ReadByte();	// armor
	int damageTaken = reader.ReadByte();	// health
	long bitsDamage = reader.ReadLong(); // damage bits

	Vector vecFrom;

	for ( int i = 0 ; i < 3 ; i++)
		vecFrom[i] = reader.ReadCoord();

	UpdateTiles(gHUD.m_flTime, bitsDamage);

	// Actually took damage?
	if ( damageTaken > 0 || armor > 0 )
	{
		CalcDamageDirection(vecFrom);
		if( g_iXash )
		{
			float time = damageTaken * 4.0f + armor * 2.0f;

			if( time > 200.0f ) time = 200.0f;
			if(gMobileAPI.pfnVibrate)
				gMobileAPI.pfnVibrate( time, 1 );
		}
	}
	return 1;
}

int CHudHealth:: MsgFunc_ScoreAttrib(const char *pszName,  int iSize, void *pbuf )
{
	BufferReader reader( pszName, pbuf, iSize );

	int index = reader.ReadByte();
	unsigned char flags = reader.ReadByte();
	g_PlayerExtraInfo[index].dead   = !!(flags & PLAYER_DEAD);
	g_PlayerExtraInfo[index].has_c4 = !!(flags & PLAYER_HAS_C4);
	g_PlayerExtraInfo[index].vip    = !!(flags & PLAYER_VIP);
	g_PlayerExtraInfo[index].zombie = !!(flags & PLAYER_ZOMBIE);
	return 1;
}
// Returns back a color from the
// Green <-> Yellow <-> Red ramp
void CHudHealth::GetPainColor(int& r, int& g, int& b, int& a)
{
#if 0
	int iHealth = m_iHealth;

	if (iHealth > 25)
		iHealth -= 25;
	else if (iHealth < 0)
		iHealth = 0;
	g = iHealth * 255 / 100;
	r = 255 - g;
	b = 0;
#else
	if (m_iHealth > 25)
	{
		DrawUtils::UnpackRGB(r, g, b, gHUD.m_hudstyle->value == 1 ? RGB_WHITE : RGB_YELLOWISH);
	}
	else
	{
		r = 250;
		g = 0;
		b = 0;
	}
	if (gHUD.m_hudstyle->value == 1)
		a = 255;
	if (m_iHealth <= 15 && gHUD.m_hudstyle->value != 1)
	{
		a = 255; // If health is getting low, make it bright red
	}
	else if (m_iHealth <= 25 && gHUD.m_hudstyle->value == 1)
	{
		a = 255;
	}
	else
	{
		// Has health changed? Flash the health #
		if (m_fFade)
		{
			m_fFade -= (gHUD.m_flTimeDelta * 20);

			if (m_fFade <= 0)
			{
				m_fFade = 0;
				if (gHUD.m_hudstyle->value != 1)
					a = MIN_ALPHA;
			}
			else
			{
				if (gHUD.m_hudstyle->value == 1)
				{
					r = 255;
					g = 255 - (m_fFade / FADE_TIME) * 255;
					b = 255 - (m_fFade / FADE_TIME) * 255;
				}
				else
					a = MIN_ALPHA +  (m_fFade/FADE_TIME) * 128; // Fade the health number back to dim
			}
		}
		else
		{
			if (gHUD.m_hudstyle->value != 1)
				a = MIN_ALPHA;
		}
	}


#endif
}


int CHudHealth::Draw(float flTime)
{
	if( !(gHUD.m_iHideHUDDisplay & HIDEHUD_HEALTH ) && !gEngfuncs.IsSpectateOnly() )
	{
		if (gHUD.m_hudstyle->value == 2)
			DrawNewHudHealth(flTime);
		else
			DrawHealthBar(flTime);
		DrawDamage(flTime);
		DrawPain(flTime);
	}

	return 1;
}

void CHudHealth::DrawNewHudHealth(float flTime)
{
	if (gHUD.m_iWeaponBits & (1 << (WEAPON_SUIT)))
	{
		int iX = 10;
		int iY = ScreenHeight - 15 - gHUD.m_NEWHUD_iFontHeight;

		int CrossWidth = gHUD.GetSpriteRect(m_NEWHUD_cross).right - gHUD.GetSpriteRect(m_NEWHUD_cross).left;
		int CrossHeight = gHUD.GetSpriteRect(m_NEWHUD_cross).bottom - gHUD.GetSpriteRect(m_NEWHUD_cross).top;

		int r = 255, g = 255, b = 255;
		if (m_iHealth <= 25)
			g = b = 0;

		SPR_Set(gHUD.GetSprite(m_NEWHUD_cross), r, g, b);
		SPR_DrawAdditive(0, iX, iY + abs(gHUD.m_NEWHUD_iFontHeight - CrossHeight) / 2, &gHUD.GetSpriteRect(m_NEWHUD_cross));

		iX += CrossWidth + 3;

		DrawUtils::DrawNEWHudNumber(0, iX, iY, m_iHealth, r, g, b, 255, FALSE, 5);
	}
}

void CHudHealth::DrawHealthBar( float flTime )
{
	int r, g, b;
	int a = 0, x, y, x1;
	int HealthWidth;
	int HealthHeight;

	GetPainColor( r, g, b, a );
	DrawUtils::ScaleColors(r, g, b, a );

	// Only draw health if we have the suit.
	if (gHUD.m_iWeaponBits & (1<<(WEAPON_SUIT)))
	{
		HealthHeight = gHUD.GetSpriteRect(gHUD.m_HUD_number_0).bottom - gHUD.GetSpriteRect(gHUD.m_HUD_number_0).top;
		HealthWidth = gHUD.GetSpriteRect(gHUD.m_HUD_number_0).right - gHUD.GetSpriteRect(gHUD.m_HUD_number_0).left;
		int CrossWidth = gHUD.GetSpriteRect(m_HUD_cross).right - gHUD.GetSpriteRect(m_HUD_cross).left;

		y = ScreenHeight - gHUD.m_iFontHeight - gHUD.m_iFontHeight / 2;
		x = CrossWidth /2;

		if (gHUD.m_hudstyle->value == 1)
		{
			if(m_iHealth <= 25)
				m_pTexture_Black->Draw2DQuadScaled(0, y - gHUD.m_iFontHeight / 2, ScreenWidth / 5 - HealthWidth, ScreenHeight, 0, 0, 1, 1, 250, 0, 0, 70);
			else
				m_pTexture_Black->Draw2DQuadScaled(0, y - gHUD.m_iFontHeight / 2, ScreenWidth / 5 - HealthWidth, ScreenHeight, 0, 0, 1, 1, 0, 0, 0, 100);
		}

		SPR_Set(gHUD.GetSprite(m_HUD_cross), r, g, b);
		SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(m_HUD_cross));

		x = CrossWidth + HealthWidth / 2;

		x = DrawUtils::DrawHudNumber(x, y, DHN_3DIGITS | DHN_DRAWZERO, m_iHealth, r, g, b);
		//x = DrawUtils::DrawHudNumber2(x, y, m_iHealth, r, g, b);



		if (gHUD.m_hudstyle->value == 1)
		{
			float f = (float)m_iHealth / (float)m_iMaxHealth;
			x = DrawBar(x + HealthWidth / 2, y + 2.5, HealthWidth * 5, HealthHeight * 0.8, f, r, g, b, a); //  height 20 number 25
		}

	}
}

int CHudHealth::DrawBar(int x, int y, int width, int height, float f, int& r, int& g, int& b, int& a)
{

	f = bound(0, f, 1);
	int w = f * width;
	if (f > 0.25)
	{
		// Always show at least one pixel if we have ammo.
		FillRGBA(x, y, w, height, r, g, b, a);
		x += w;
		width -= w;
	}
	else
	{
		if (w <= 0)
			w = 1;
		FillRGBA(x, y, w, height, r, g, b, a);
		x += w;
		width -= w;
	}

	return (x + width);
}

void CHudHealth::CalcDamageDirection( Vector vecFrom )
{
	Vector	forward, right, up;
	float	side, front, flDistToTarget;

	if( vecFrom.IsZero() )
	{
		for( int i = 0; i < 4; i++ )
			m_fAttack[i] = 0;
		return;
	}

	vecFrom = vecFrom - gHUD.m_vecOrigin;
	flDistToTarget = vecFrom.Length();
	vecFrom = vecFrom.Normalize();
	AngleVectors (gHUD.m_vecAngles, forward, right, up);

	front = DotProduct (vecFrom, right);
	side = DotProduct (vecFrom, forward);

	if (flDistToTarget <= 50)
	{
		for( int i = 0; i < 4; i++ )
			m_fAttack[i] = 1;
	}
	else
	{
		if (side > EPSILON)
			m_fAttack[0] = max(m_fAttack[0], side);
		if (side < -EPSILON)
			m_fAttack[1] = max(m_fAttack[1], 0 - side );
		if (front > EPSILON)
			m_fAttack[2] = max(m_fAttack[2], front);
		if (front < -EPSILON)
			m_fAttack[3] = max(m_fAttack[3], 0 - front );
	}
}

void CHudHealth::DrawPain(float flTime)
{
	if (m_fAttack[0] == 0 &&
		m_fAttack[1] == 0 &&
		m_fAttack[2] == 0 &&
		m_fAttack[3] == 0)
		return;

	float a, fFade = gHUD.m_flTimeDelta * 2;

	for( int i = 0; i < 4; i++ )
	{
		if( m_fAttack[i] > EPSILON )
		{
			/*GetPainColor(r, g, b);
			shade = a * max( m_fAttack[i], 0.5 );
			DrawUtils::ScaleColors(r, g, b, shade);*/

			a = max( m_fAttack[i], 0.5f );

			SPR_Set( m_hSprite, 255 * a, 255 * a, 255 * a);
			SPR_DrawAdditive( i, m_vAttackPos[i].x, m_vAttackPos[i].y, NULL );
			m_fAttack[i] = max( 0.0f, m_fAttack[i] - fFade );
		}
		else
			m_fAttack[i] = 0;
	}
}

void CHudHealth::DrawDamage(float flTime)
{
	int r, g, b, a;

	if (!m_bitsDamage)
		return;

	DrawUtils::UnpackRGB(r,g,b, RGB_YELLOWISH);

	a = (int)( fabs(sin(flTime*2)) * 256.0);

	DrawUtils::ScaleColors(r, g, b, a);
	size_t i;
	// Draw all the items
	for (i = 0; i < NUM_DMG_TYPES; i++)
	{
		if (m_bitsDamage & giDmgFlags[i])
		{
			DAMAGE_IMAGE *pdmg = &m_dmg[i];
			SPR_Set(gHUD.GetSprite(m_HUD_dmg_bio + i), r, g, b );
			SPR_DrawAdditive(0, pdmg->x, pdmg->y, &gHUD.GetSpriteRect(m_HUD_dmg_bio + i));
		}
	}


	// check for bits that should be expired
	for ( i = 0; i < NUM_DMG_TYPES; i++ )
	{
		DAMAGE_IMAGE *pdmg = &m_dmg[i];

		if ( m_bitsDamage & giDmgFlags[i] )
		{
			pdmg->fExpire = min( flTime + DMG_IMAGE_LIFE, pdmg->fExpire );

			if ( pdmg->fExpire <= flTime		// when the time has expired
				 && a < 40 )						// and the flash is at the low point of the cycle
			{
				pdmg->fExpire = 0;

				int y = pdmg->y;
				pdmg->x = pdmg->y = 0;

				// move everyone above down
				for (size_t j = 0; j < NUM_DMG_TYPES; j++)
				{
					pdmg = &m_dmg[j];
					if ((pdmg->y) && (pdmg->y < y))
						pdmg->y += giDmgHeight;

				}

				m_bitsDamage &= ~giDmgFlags[i];  // clear the bits
			}
		}
	}
}


void CHudHealth::UpdateTiles(float flTime, long bitsDamage)
{
	DAMAGE_IMAGE *pdmg;

	// Which types are new?
	long bitsOn = ~m_bitsDamage & bitsDamage;

	for (size_t i = 0; i < NUM_DMG_TYPES; i++)
	{
		pdmg = &m_dmg[i];

		// Is this one already on?
		if (m_bitsDamage & giDmgFlags[i])
		{
			pdmg->fExpire = flTime + DMG_IMAGE_LIFE; // extend the duration
			if (!pdmg->fBaseline)
				pdmg->fBaseline = flTime;
		}

		// Are we just turning it on?
		if (bitsOn & giDmgFlags[i])
		{
			// put this one at the bottom
			pdmg->x = giDmgWidth/8;
			pdmg->y = ScreenHeight - giDmgHeight * 2;
			pdmg->fExpire=flTime + DMG_IMAGE_LIFE;

			// move everyone else up
			for (size_t j = 0; j < NUM_DMG_TYPES; j++)
			{
				if (j == i)
					continue;

				pdmg = &m_dmg[j];
				if (pdmg->y)
					pdmg->y -= giDmgHeight;

			}
			pdmg = &m_dmg[i];
		}
	}

	// damage bits are only turned on here;  they are turned off when the draw time has expired (in DrawDamage())
	m_bitsDamage |= bitsDamage;
}


int CHudHealth :: MsgFunc_ClCorpse(const char *pszName, int iSize, void *pbuf)
{
#if 0
	BufferReader reader(pbuf, iSize);

	char szModel[64];

	char *pModel = reader.ReadString();
	Vector origin;
	origin.x = reader.ReadLong() / 128.0f;
	origin.y = reader.ReadLong() / 128.0f;
	origin.z = reader.ReadLong() / 128.0f;
	Vector angles;
	angles.x = reader.ReadCoord();
	angles.y = reader.ReadCoord();
	angles.z = reader.ReadCoord();
	float delay = reader.ReadLong() / 100.0f;
	int sequence = reader.ReadByte();
	int classID = reader.ReadByte();
	int teamID = reader.ReadByte();
	int playerID = reader.ReadByte();

	if( !cl_minmodels->value )
	{
		if( !strstr(pModel, "models/") )
		{
			snprintf(szModel, sizeof(szModel), "models/player/%s/%s.mdl", pModel, pModel );
		}
	}
	else if( teamID == 1 ) // terrorists
	{
		int modelidx = cl_min_t->value;
		if( BIsValidTModelIndex(modelidx) )
			strncpy(szModel, sPlayerModelFiles[modelidx], sizeof(szModel));
		else strncpy(szModel, sPlayerModelFiles[1], sizeof(szModel) ); // set leet.mdl
	}
	else if( teamID == 2 ) // ct
	{
		int modelidx = cl_min_ct->value;

		if( g_PlayerExtraInfo[playerID].vip )
			strncpy( szModel, sPlayerModelFiles[3], sizeof(szModel) ); // vip.mdl
		else if( BIsValidCTModelIndex( modelidx ) )
			strncpy( szModel, sPlayerModelFiles[ modelidx ], sizeof(szModel));
		else strncpy( szModel, sPlayerModelFiles[2], sizeof(szModel) ); // gign.mdl
	}
	else strncpy( szModel, sPlayerModelFiles[0], sizeof(szModel) ); // player.mdl

	CreateCorpse( &origin, &angles, szModel, delay, sequence, classID );
#endif
   return 0;
}

/*
============
CL_IsDead

Returns 1 if health is <= 0
============
*/
bool CL_IsDead()
{
	if( gHUD.m_Health.m_iHealth <= 0 )
		return true;
	return false;
}

}