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

ConstSprList s_ModelY23S3BuffM249_V1 = {
	"v_y23s3buffm249.mdl",
	"$0b_y23s3buffm249_ef.bmp",
	"sprites/ef_y23s3buffm249_v.spr",
	30.0,
	{
		{ 0, true, false, 1.0, 0, 0, 0 },
		{ 1, true, false, 1.0, 0, 0, 0 },
		{ 2, true, false, 1.0, 0, 0, 0 },
		{ 3, true, false, 1.0, 0, 0, 0 },
		{ 4, true, false, 1.0, 0, 0, 0 },
		{ 5, true, false, 1.0, 0, 0, 0 },
	}
};

ConstSprList s_ModelY23S3BuffM249_V2 = {
	"v_y23s3buffm249.mdl",
	"$0b_y23s3buffm249_ef2.bmp",
	"sprites/ef_y23s3buffm249_v2.spr",
	30.0,
	{
		{ 0, true, false, 1.0, 0, 0, 0 },
		{ 1, true, false, 1.0, 0, 0, 0 },
		{ 2, true, false, 1.0, 0, 0, 0 },
		{ 3, true, false, 1.0, 0, 0, 0 },
		{ 4, true, false, 1.0, 0, 0, 0 },
		{ 5, true, false, 1.0, 0, 0, 0 },
	}
};

ConstSprList s_ModelIgniteBombFire = {
	"ef_ignitebomb.mdl",
	"$0b_ignitebomb_bffire.BMP",
	"sprites/ef_ignite_bomb_fire.spr",
	30.0,
	{
		{ 0, false, true, 1.0, 0, 0, 0 },
	}
};

ConstSprList s_ModelEFIgniteBombLine = {
	"ef_ignitebomb.mdl",
	"$0b_ignitebomb_bfxline.BMP",
	"sprites/ef_ignite_bomb_line.spr",
	30.0,
	{
		{ 0, false, true, 1.0, 0, 0, 0 },
	}
};

ConstSprList s_ModelLinkAttackSG_Oil01 = {
	"v_linkattacksg.mdl",
	"$20_linkattacksg_oil01.bmp",
	"sprites/ef_linkattacksg_oil01.spr",
	30.0,
	{
		{ 0, true, false, 1.0, 0, 0, 0 },
		{ 1, true, false, 1.0, 0, 0, 0 },
		{ 2, true, false, 1.0, 0, 0, 0 },
		{ 3, true, false, 1.0, 0, 0, 0 },
		{ 4, true, false, 1.0, 0, 0, 0 },
		{ 5, true, false, 1.0, 0, 0, 0 },
		{ 6, true, false, 1.0, 0, 0, 0 },
		{ 7, true, false, 1.0, 0, 0, 0 },
	}
};

ConstSprList s_ModelLinkAttackSG_Laser01 = {
	"v_linkattacksg.mdl",
	"$0b_linkattacksg_laser01.bmp",
	"sprites/ef_linkattacksg_laser01.spr",
	30.0,
	{
		{ 0, true, false, 1.0, 0, 0, 0 },
		{ 1, true, false, 1.0, 0, 0, 0 },
		{ 2, true, false, 1.0, 0, 0, 0 },
		{ 3, true, false, 1.0, 0, 0, 0 },
		{ 4, true, false, 1.0, 0, 0, 0 },
		{ 5, true, false, 1.0, 0, 0, 0 },
		{ 6, true, false, 1.0, 0, 0, 0 },
		{ 7, true, false, 1.0, 0, 0, 0 },
	}
};


ConstSprList s_ModelLinkAttackSG_Sphere_V = {
	"v_linkattacksg.mdl",
	"$0b_linkattacksg_sphere_v.bmp",
	"sprites/ef_linkattacksg_sphere_v.spr",
	30.0,
	{
		{ 0, true, false, 1.0, 0, 0, 0 },
		{ 1, true, false, 1.0, 0, 0, 0 },
		{ 2, true, false, 1.0, 0, 0, 0 },
		{ 3, true, false, 1.0, 0, 0, 0 },
		{ 4, true, false, 1.0, 0, 0, 0 },
		{ 5, true, false, 1.0, 0, 0, 0 },
		{ 6, true, false, 1.0, 0, 0, 0 },
		{ 7, true, false, 1.0, 0, 0, 0 },
	}
};


ConstSprList s_ModelLinkAttackSG_Oil02 = {
	"v_linkattacksg.mdl",
	"$20_linkattacksg_oil02.bmp",
	"sprites/ef_linkattacksg_oil02.spr",
	30.0,
	{
		{ 0, true, false, 1.0, 0, 0, 0 },
		{ 1, true, false, 1.0, 0, 0, 0 },
		{ 2, true, false, 1.0, 0, 0, 0 },
		{ 3, true, false, 1.0, 0, 0, 0 },
		{ 4, true, false, 1.0, 0, 0, 0 },
		{ 5, true, false, 1.0, 0, 0, 0 },
		{ 6, true, false, 1.0, 0, 0, 0 },
		{ 7, true, false, 1.0, 0, 0, 0 },
	}
};

ConstSprList s_ModelLinkAttackSG_Laser02 = {
	"v_linkattacksg.mdl",
	"$0b_linkattacksg_laser02.bmp",
	"sprites/ef_linkattacksg_laser02.spr",
	30.0,
	{
		{ 0, true, false, 1.0, 0, 0, 0 },
		{ 1, true, false, 1.0, 0, 0, 0 },
		{ 2, true, false, 1.0, 0, 0, 0 },
		{ 3, true, false, 1.0, 0, 0, 0 },
		{ 4, true, false, 1.0, 0, 0, 0 },
		{ 5, true, false, 1.0, 0, 0, 0 },
		{ 6, true, false, 1.0, 0, 0, 0 },
		{ 7, true, false, 1.0, 0, 0, 0 },
	}
};

ConstSprList s_ModelLinkAttackSGPart_Oil01 = {
	"v_linkattacksg_parts.mdl",
	"$20_linkattacksg_oil01.bmp",
	"sprites/ef_linkattacksg_oil01.spr",
	30.0,
	{
		{ 0, true, false, 1.0, 0, 0, 0 },
		{ 1, true, false, 1.0, 0, 0, 0 },
		{ 2, true, false, 1.0, 0, 0, 0 },
		{ 3, true, false, 1.0, 0, 0, 0 },
	}
};

ConstSprList s_ModelLinkAttackSGPart_Oil02 = {
	"v_linkattacksg_parts.mdl",
	"$20_linkattacksg_oil02.bmp",
	"sprites/ef_linkattacksg_oil02.spr",
	30.0,
	{
		{ 0, true, false, 1.0, 0, 0, 0 },
		{ 1, true, false, 1.0, 0, 0, 0 },
		{ 2, true, false, 1.0, 0, 0, 0 },
		{ 3, true, false, 1.0, 0, 0, 0 },
	}
};

ConstSprList s_ModelLinkAttackSGField_Sphere01 = {
	"ef_linkattacksg_field.mdl",
	"$0b_linkattacksg_sphere01.bmp",
	"sprites/ef_linkattacksg_sphere01.spr",
	15.0,
	{
		{ 0, true, false, 1.0, 0, 0, 0 },
	}
};

ConstSprList s_ModelLinkAttackSGField_Sphere02 = {
	"ef_linkattacksg_field.mdl",
	"$0b_linkattacksg_sphere02.bmp",
	"sprites/ef_linkattacksg_sphere02.spr",
	15.0,
	{
		{ 0, true, false, 1.0, 0, 0, 0 },
	}
};

ConstSprList s_ModelLinkAttackSGField_Sphere03 = {
	"ef_linkattacksg_field.mdl",
	"$0b_linkattacksg_sphere03.bmp",
	"sprites/ef_linkattacksg_sphere03.spr",
	50.0,
	{
		{ 0, true, false, 1.0, 0, 0, 0 },
	}
};

ConstSprList s_ModelBuffClass23S3CT = {
	"buffclass23s3ct.mdl",
	"buffclass23s3ct.bmp",
	"sprites/ef_buffclass23s3ct.spr",
	30.0,
	{
		{ 200, false, true, 1.0, 0, 0, 0 },
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
	LoadSpr(&s_ModelY23S3BuffM249_V1);
	LoadSpr(&s_ModelY23S3BuffM249_V2);
	LoadSpr(&s_ModelIgniteBombFire);
	LoadSpr(&s_ModelEFIgniteBombLine);
	LoadSpr(&s_ModelLinkAttackSG_Oil01);
	LoadSpr(&s_ModelLinkAttackSG_Laser01);
	LoadSpr(&s_ModelLinkAttackSG_Sphere_V);
	LoadSpr(&s_ModelLinkAttackSG_Oil02);
	LoadSpr(&s_ModelLinkAttackSG_Laser02);
	LoadSpr(&s_ModelLinkAttackSGPart_Oil01);
	LoadSpr(&s_ModelLinkAttackSGPart_Oil02);
	LoadSpr(&s_ModelLinkAttackSGField_Sphere01);
	LoadSpr(&s_ModelLinkAttackSGField_Sphere02);
	LoadSpr(&s_ModelLinkAttackSGField_Sphere03);
	LoadSpr(&s_ModelBuffClass23S3CT);
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
