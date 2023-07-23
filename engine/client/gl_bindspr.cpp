/*
gl_studiolru.h
Copyright (C) 2023 Moemod Hymei

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "common.h"
#include "client.h"
#include "gl_local.h"
#include "gl_bindspr.h"
#include "studio.h"

#include "gl_studioshader.h"

ConstSprList s_ModelLeapStrikeGunFlying = {
	"ef_leapstrikegun_flying.mdl",
	"$0b_leapstrikegun_flying.bmp",
	"sprites/ef_leapstrikegun_fly.spr",
	30.0,
	{
		{ 0, true, false, 1.0, 0, 0, 0 }
	}
};

ConstSprList s_ModelLeapStrikeGun = {
	"v_leapstrikegun.mdl",
	"$0b_leapstrikegun_v_effect01.bmp",
	"sprites/leapstrikegun_v_ef.spr",
	30.0,
	{
		{ 0, true, false, 1.0, 0, 0, 0 },
		{ 1, true, false, 1.0, 0, 0, 0 },
		{ 2, true, false, 1.0, 0, 0, 0 },
		{ 3, true, false, 1.0, 0, 0, 0 },
		{ 4, true, false, 1.0, 0, 0, 0 }
	}
};

ConstSprList s_ModelLeapStrikeGunPart = {
	"v_leapstrikegun_parts.mdl",
	"$0b_leapstrikegun_v_effect01.bmp",
	"sprites/leapstrikegun_v_ef.spr",
	30.0,
	{
		{ 0, true, false, 1.0, 0, 0, 0 },
		{ 1, true, false, 1.0, 0, 0, 0 },
		{ 2, true, false, 1.0, 0, 0, 0 },
		{ 3, true, false, 1.0, 0, 0, 0 }
	}
};

ConstSprList s_ModelLeapStrikeGunPartFront = {
	"v_leapstrikegun_parts.mdl",
	"$0b_leapstrikegun_bmodevf.bmp",
	"sprites/ef_leapstrikegun_bmode_front.spr",
	30.0,
	{
		{ 0, true, true, 1.0, 0, 1, 0 },
		{ 1, false, true, 1.0, 1, 30, 0 },
		{ 2, false, true, 1.0, 31, 60, 0 },
		{ 3, false, true, 1.0, 61, 74, 0 },
	}
};

ConstSprList s_ModelEFDvHammer1 = {
	"ef_dvhammer2.mdl",
	"$0b_dvhammer_ef2_explo.bmp",
	"sprites/ef_dvhammer_explo_b1.spr",
	30.0,
	{
		{ 0, false, true, 1.0, 0, 0, 0 }
	}
};

ConstSprList s_ModelEFDvHammer2 = {
	"ef_dvhammer2.mdl",
	"$0b_dvhammer_ef2_lightning.bmp",
	"sprites/ef_dvhammer_lightning_b1.spr",
	30.0,
	{
		{ 0, false, true, 1.0, 0, 0, 0 }
	}
};

ConstSprList s_ModelLeapStrikeGunJumpDash2 = {
	"ef_leapstrikegun_jumpdash.mdl",
	"$0b_leapstrikegun_jumpdash2.bmp",
	"sprites/ef_leapstrikegun_jumpdash2.spr",
	30.0,
	{
		{ 0, true, false, 1.0, 0, 0, 0 }
	}
};

ConstSprList s_ModelLeapStrikeGunJumpDash = {
	"ef_leapstrikegun_jumpdash.mdl",
	"0b_leapstrikegun_jumpdash.bmp",
	"sprites/ef_leapstrikegun_jumpdash.spr",
	30.0,
	{
		{ 0, true, false, 1.0, 0, 0, 0 }
	}
};

ConstSprList s_ModelLeapStrikeGunLandingFX = {
	"ef_leapstrikegun_landing.mdl",
	"$0b_leapstrikegun_jumpfx.bmp",
	"sprites/ef_leapstrikegun_jump.spr",
	30.0,
	{
		{ 0, false, true, 1.0, 0, 0, 0 }
	}
};

ConstSprList s_ModelLeapStrikeGunLandingGround = {
	"ef_leapstrikegun_landing.mdl",
	"$0a_leapstrikegun_lendexplo_ground.bmp",
	"sprites/ef_leapstrikegun_landexplo_ground.spr",
	30.0,
	{
		{ 0, false, true, 1.0, 0, 0, 0 }
	}
};

ConstSprList s_ModelLeapStrikeGunLandingEXP1 = {
	"ef_leapstrikegun_landing.mdl",
	"$0b_leapstrikegun_lendexplo1.bmp",
	"sprites/leapstrikegun_lendexplo.spr",
	30.0,
	{
		{ 0, false, true, 1.0, 0, 0, 0 }
	}
};

ConstSprList s_ModelLeapStrikeGunLandingEXP2 = {
	"ef_leapstrikegun_landing.mdl",
	"$0b_leapstrikegun_lendexplo2.bmp",
	"sprites/leapstrikegun_lendexplo2.spr",
	30.0,
	{
		{ 0, false, true, 1.0, 0, 0, 0 }
	}
};

ConstSprList s_ModelLeapStrikeGunLandingFallTail = {
	"ef_leapstrikegun_fall.mdl",
	"$0b_leapstrikegun_airdrop1.bmp",
	"sprites/ef_leapstrikegun_fall_tail.spr",
	30.0,
	{
		{ 0, true, false, 1.0, 0, 0, 0 }
	}
};

ConstSprList s_ModelLeapStrikeGunFallHead = {
	"ef_leapstrikegun_fall.mdl",
	"$0b_leapstrikegun_airdrop2.bmp",
	"sprites/ef_leapstrikegun_fall_head.spr",
	30.0,
	{
		{ 0, true, false, 1.0, 0, 0, 0 }
	}
};

ConstSprList s_ModelLeapStrikeGunJumpFX = {
	"ef_leapstrikegun_jumpfx.mdl",
	"$0b_leapstrikegun_jumpfx.bmp",
	"sprites/ef_leapstrikegun_jump.spr",
	30.0,
	{
		{ 0, true, false, 1.0, 0, 0, 0 }
	}
};

ConstSprList s_ModelLeapStrikeGunJumpFX2 = {
	"ef_leapstrikegun_jumpfx.mdl",
	"$0b_leapstrikegun_jumpfx2.bmp",
	"sprites/ef_leapstrikegun_jump2.spr",
	30.0,
	{
		{ 0, true, false, 1.0, 0, 0, 0 }
	}
};

ConstSprList s_ModelM3DragonEX = {
	"ef_m3dragonex.mdl",
	"$2a_m3dragonex_effecthead.bmp",
	"sprites/ef_m3dragonex.spr",
	30.0,
	{
		{ 0, true, false, 1.0, 0, 0, 0 }
	}
};

ConstSprList s_ModelM3DragonEXSign = {
	"ef_m3dragonex_sign.mdl",
	"$0a_ef_m3dragonex_sign.bmp",
	"sprites/ef_m3dragonex_sign.spr",
	30.0,
	{
		{ 0, true, false, 1.0, 0, 0, 0 }
	}
};

ConstSprList s_ModelLeapStrikeGunMuzz = {
	"v_leapstrikegun.mdl",
	"$0b_leapstrikegun_muzzle.bmp",
	"sprites/ef_leapstrikegun_vmuzzle.spr",
	30.0,
	{
		{ 1, false, false, 1.0, 0, 0, 0 },
		{ 2, false, false, 1.0, 0, 0, 0 }
	}
};

ConstSprList s_ModelLeapStrikeGunShoot = {
	"v_leapstrikegun.mdl",
	"$0b_leapstrikegun_v_effect012.bmp",
	"sprites/ef_leapstrikegun_shoot.spr",
	30.0,
	{
		{ 1, false, true, 1.0, 0, 0, 0 },
		{ 2, false, true, 1.0, 0, 0, 0 }
	}
};

std::vector<SprList> g_vecBindSprList;

static CBindSprManager g_SprBindManager;

CBindSprManager& BindSprManager(void)
{
	return g_SprBindManager;
}

void CBindSprManager::Vidinit(void)
{
	g_vecBindSprList.clear();

	LoadSpr(&s_ModelLeapStrikeGunFlying);
	LoadSpr(&s_ModelEFDvHammer1);
	LoadSpr(&s_ModelEFDvHammer2);
	LoadSpr(&s_ModelLeapStrikeGun);
	LoadSpr(&s_ModelLeapStrikeGunPart);
	LoadSpr(&s_ModelLeapStrikeGunPartFront);
	LoadSpr(&s_ModelLeapStrikeGunMuzz);
	LoadSpr(&s_ModelLeapStrikeGunShoot);
	LoadSpr(&s_ModelLeapStrikeGunJumpDash);
	LoadSpr(&s_ModelLeapStrikeGunJumpDash2);
	LoadSpr(&s_ModelLeapStrikeGunLandingFX);
	LoadSpr(&s_ModelLeapStrikeGunLandingGround);
	LoadSpr(&s_ModelLeapStrikeGunLandingEXP1);
	LoadSpr(&s_ModelLeapStrikeGunLandingEXP2);
	LoadSpr(&s_ModelLeapStrikeGunLandingFallTail);
	LoadSpr(&s_ModelLeapStrikeGunFallHead);
	LoadSpr(&s_ModelLeapStrikeGunJumpFX);
	LoadSpr(&s_ModelLeapStrikeGunJumpFX2);
	LoadSpr(&s_ModelM3DragonEX);
	LoadSpr(&s_ModelM3DragonEXSign);
}

int CBindSprManager::LoadSpr(ConstSprList *List)
{
	if (List == nullptr)
		return 0;

	g_vecBindSprList.emplace_back();
	SprList& Temp = g_vecBindSprList.back();

	sprintf(Temp.mdlname, "%s", List->mdlname);
	sprintf(Temp.texname, "%s", List->texname);
	sprintf(Temp.sprname, "%s", List->sprname);

	Temp.sprFPS = List->sprFPS;
	Temp.vecSequence = List->vecSequence;
	return 1;
}
