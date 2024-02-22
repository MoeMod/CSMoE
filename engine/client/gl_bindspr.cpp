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

ConstSprList s_ModelEFDvHammer2 = {
	"ef_dvhammer2.mdl",
	{
		{
			0,
			"sprites/ef_dvhammer_explo_b1.spr",
			30,
			{
				{ 0, false, true, 1.0, 0, 0, 0 }
			}
		},
		{
			1,
			"sprites/ef_dvhammer_lightning_b1.spr",
			30,
			{
				{ 0, false, true, 1.0, 0, 0, 0 }
			}
		},
	}
};

ConstSprList s_ModelEFIgniteBomb = {
	"ef_ignitebomb.mdl",
	{
		{
			0,
			"sprites/ef_ignite_bomb_fire.spr",
			30,
			{
				{ 0, false, true, 1.0, 0, 0, 0 }
			}
		},
		{
			1,
			"sprites/ef_ignite_bomb_line.spr",
			30,
			{
				{ 0, false, true, 1.0, 0, 0, 0 }
			}
		},
	}
};

ConstSprList s_ModelEFLeapStrikeGun_Fall = {
	"ef_leapstrikegun_fall.mdl",
	{
		{
			0,
			"sprites/ef_leapstrikegun_fall_tail.spr",
			30,
			{
				{ 0, true, false, 1.0, 0, 0, 0 }
			}
		},
		{
			1,
			"sprites/ef_leapstrikegun_fall_head.spr",
			30,
			{
				{ 0, true, false, 1.0, 0, 0, 0 }
			}
		},
	}
};

ConstSprList s_ModelEFLeapStrikeGunFlying = {
	"ef_leapstrikegun_flying.mdl",
	{
		{
			0,
			"sprites/ef_leapstrikegun_fly.spr",
			30,
			{
				{ 0, true, false, 1.0, 0, 0, 0 }
			}
		}
	},
};

ConstSprList s_ModelEFLeapStrikeGun_JumpDash = {
	"ef_leapstrikegun_fall.mdl",
	{
		{
			0,
			"sprites/ef_leapstrikegun_jumpdash2.spr",
			30,
			{
				{ 0, true, false, 1.0, 0, 0, 0 }
			}
		},
		{
			1,
			"sprites/ef_leapstrikegun_jumpdash.spr",
			30,
			{
				{ 0, true, false, 1.0, 0, 0, 0 }
			}
		},
	}
};

ConstSprList s_ModelEFLeapStrikeGunJump_FX = {
	"ef_leapstrikegun_fall.mdl",
	{
		{
			0,
			"sprites/ef_leapstrikegun_jump.spr",
			30,
			{
				{ 0, true, false, 1.0, 0, 0, 0 }
			}
		},
		{
			1,
			"sprites/ef_leapstrikegun_jump2.spr",
			30,
			{
				{ 0, true, false, 1.0, 0, 0, 0 }
			}
		},
	}
};

ConstSprList s_ModelEFLeapStrikeGun_Landing = {
	"ef_leapstrikegun_landing.mdl",
	{
		{
			0,
			"sprites/ef_leapstrikegun_jump.spr",
			30,
			{
				{ 0, false, true, 1.0, 0, 0, 0 }
			}
		},
		{
			1,
			"sprites/ef_leapstrikegun_landexplo_ground.spr",
			30,
			{
				{ 0, false, true, 1.0, 0, 0, 0 }
			}
		},
		{
			2,
			"sprites/leapstrikegun_lendexplo.spr",
			30.0,
			{
				{ 0, false, true, 1.0, 0, 0, 0 }
			}
		},
		{
			3,
			"sprites/leapstrikegun_lendexplo2.spr",
			30.0,
			{
				{ 0, false, true, 1.0, 0, 0, 0 }
			}
		}
	},
};

ConstSprList s_ModelEFLinkAttackSG_Field= {
	"ef_linkattacksg_field.mdl",
	{
		{
			0,
			"sprites/ef_linkattacksg_sphere01.spr",
			15,
			{
				{ 0, true, false, 1.0, 0, 0, 0 },
			}
		},
		{
			1,
			"sprites/ef_linkattacksg_sphere02.spr",
			30,
			{
				{ 0, true, false, 1.0, 0, 0, 0 },
			}
		},
		{
			2,
			"sprites/ef_linkattacksg_sphere03.spr",
			30.0,
			{
				{ 0, false, true, 1.0, 0, 0, 0 }
			}
		},
		{
			3,
			"sprites/ef_linkattacksg_sphere03.spr",
			30.0,
			{
				{ 0, true, false, 1.0, 0, 0, 0 },
			}
		}
	},
};

ConstSprList s_ModelEFM3DragonEX = {
	"ef_m3dragonex.mdl",
	{
		{
			0,
			"sprites/ef_m3dragonex.spr",
			30,
			{
				{ 0, true, false, 1.0, 0, 0, 0 }
			}
		},
	}
};

ConstSprList s_ModelEFM3DragonEX_Sign = {
	"ef_m3dragonex_sign.mdl",
	{
		{
			0,
			"sprites/ef_m3dragonex_sign.spr",
			30,
			{
				{ 0, true, false, 1.0, 0, 0, 0 }
			}
		},
	}
};

ConstSprList s_ModelLeapStrikeGun = {
	"v_leapstrikegun.mdl",
	{
		{
			4,
			"sprites/leapstrikegun_v_ef.spr",
			30,
			{
				{ 0, true, false, 1.0, 0, 0, 0 },
				{ 3, true, false, 1.0, 0, 0, 0 },
				{ 4, true, false, 1.0, 0, 0, 0 }
			}
		},
		{
			3,
			"sprites/ef_leapstrikegun_vmuzzle.spr",
			30,
			{
				{ 1, false, false, 1.0, 0, 0, 0 },
				{ 2, false, false, 1.0, 0, 0, 0 }
			}
		},
		{
			7,
			"sprites/ef_leapstrikegun_shoot.spr",
			30.0,
			{
				{ 1, false, true, 1.0, 0, 0, 0 },
				{ 2, false, true, 1.0, 0, 0, 0 }
			}
		}
	},
};

ConstSprList s_ModelLeapStrikeGunPart = {
	"v_leapstrikegun_parts.mdl",
	{
		{
			3,
			"sprites/ef_leapstrikegun_bmode_front.spr",
			30,
			{
				{ 0, true, true, 1.0, 0, 1, 0 },
				{ 1, false, true, 1.0, 1, 30, 0 },
				{ 2, false, true, 1.0, 31, 60, 0 },
				{ 3, false, true, 1.0, 61, 74, 0 }
			}
		},
		{
			4,
			"sprites/leapstrikegun_v_ef.spr",
			30,
			{
				{ 0, true, false, 1.0, 0, 0, 0 },
				{ 1, true, false, 1.0, 0, 0, 0 },
				{ 2, true, false, 1.0, 0, 0, 0 },
				{ 3, true, false, 1.0, 0, 0, 0 }
			}
		},
	},
};

ConstSprList s_ModelLinkAttackSG = {
	"v_linkattacksg.mdl",
	{
		{
			2,
			"sprites/ef_linkattacksg_oil01.spr",
			30,
			{
				{ 0, true, false, 1.0, 0, 0, 0 },
				{ 1, true, false, 1.0, 0, 0, 0 },
				{ 2, true, false, 1.0, 0, 0, 0 },
				{ 3, true, false, 1.0, 0, 0, 0 },
				{ 4, true, false, 1.0, 0, 0, 0 },
				{ 5, true, false, 1.0, 0, 0, 0 },
				{ 6, true, false, 1.0, 0, 0, 0 },
				{ 7, true, false, 1.0, 0, 0, 0 }
			}
		},
		{
			3,
			"sprites/ef_linkattacksg_laser01.spr",
			30,
			{
				{ 0, true, false, 1.0, 0, 0, 0 },
				{ 1, true, false, 1.0, 0, 0, 0 },
				{ 2, true, false, 1.0, 0, 0, 0 },
				{ 3, true, false, 1.0, 0, 0, 0 },
				{ 4, true, false, 1.0, 0, 0, 0 },
				{ 5, true, false, 1.0, 0, 0, 0 },
				{ 6, true, false, 1.0, 0, 0, 0 },
				{ 7, true, false, 1.0, 0, 0, 0 }
			}
		},
		{
			4,
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
				{ 7, true, false, 1.0, 0, 0, 0 }
			}
		},
		{
			8,
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
				{ 7, true, false, 1.0, 0, 0, 0 }
			}
		},
		{
			9,
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
				{ 7, true, false, 1.0, 0, 0, 0 }
			}
		}
	},
};

ConstSprList s_ModelLinkAttackSGPart = {
	"v_linkattacksg_parts.mdl",
	{
		{
			1,
			"sprites/ef_linkattacksg_oil01.spr",
			30,
			{
				{ 0, true, false, 1.0, 0, 0, 0 },
				{ 1, true, false, 1.0, 0, 0, 0 },
				{ 2, true, false, 1.0, 0, 0, 0 },
				{ 3, true, false, 1.0, 0, 0, 0 }
			}
		},
		{
			2,
			"sprites/ef_linkattacksg_oil02.spr",
			30,
			{
				{ 0, true, false, 1.0, 0, 0, 0 },
				{ 1, true, false, 1.0, 0, 0, 0 },
				{ 2, true, false, 1.0, 0, 0, 0 },
				{ 3, true, false, 1.0, 0, 0, 0 }
			}
		}
	},
};

ConstSprList s_ModelY23S3BuffM249 = {
	"v_y23s3buffm249.mdl",
	{
		{
			1,
			"sprites/ef_y23s3buffm249_v.spr",
			30,
			{
				{ 0, true, false, 1.0, 0, 0, 0 },
				{ 1, true, false, 1.0, 0, 0, 0 },
				{ 2, true, false, 1.0, 0, 0, 0 },
				{ 3, true, false, 1.0, 0, 0, 0 },
				{ 4, true, false, 1.0, 0, 0, 0 },
				{ 5, true, false, 1.0, 0, 0, 0 }
			}
		},
		{
			6,
			"sprites/ef_y23s3buffm249_v2.spr",
			30,
			{
				{ 0, true, false, 1.0, 0, 0, 0 },
				{ 1, true, false, 1.0, 0, 0, 0 },
				{ 2, true, false, 1.0, 0, 0, 0 },
				{ 3, true, false, 1.0, 0, 0, 0 },
				{ 4, true, false, 1.0, 0, 0, 0 },
				{ 5, true, false, 1.0, 0, 0, 0 }
			}
		}
	},
};

ConstSprList s_ModelBuffClass23S3CT = {
	"buffclass23s3ct.mdl",
	{
		{
			0,
			"sprites/ef_buffclass23s3ct.spr",
			30,
			{
				{ 200, false, true, 1.0, 0, 0, 0 }
			}
		},
	}
};

ConstSprList s_ModelSoulReaperShootC02 = {
	"ef_soulreaper_shoot_c02.mdl",
	{
		{
			0,
			"sprites/ef_soulreaper_shoot_c.spr",
			15,
			{
				{ 0, true, false, 1.0, 0, 0, 0 },
			}
		},
	}
};

ConstSprList s_ModelSoulReaperShootC = {
	"ef_soulreaper_shoot_c.mdl",
	{
		{
			0,
			"sprites/ef_soulreaper_shoot_c.spr",
			15,
			{
				{ 0, true, false, 1.0, 0, 0, 0 },
			}
		},
	}
};

ConstSprList s_ModelSoulReaperSkill = {
	"ef_soulreaper_skill.mdl",
	{
		{
			0,
			"sprites/ef_soulreaper_skill.spr",
			30,
			{
				{ 0, false, true, 1.0, 0, 0, 0 },
			}
		},
	}
};

ConstSprList s_ModelPSoulReaper = {
	"p_soulreaper.mdl",
	{
		{
			1,
			"sprites/ef_soulreaper_skill.spr",
			30,
			{
				{ 2, false, true, 1.0, 0, 0, 0 },
			}
		},
	}
};

ConstSprList s_ModelSoulReaperAmode = {
	"ef_soulreaper_Amode.mdl",
	{
		{
			0,
			"sprites/ef_soulreaper_p.spr",
			30,
			{
				{ 0, false, true, 1.0, 0, 0, 0 },
			}
		},
	}
};

ConstSprList s_ModelSoulReaperCmode = {
	"ef_soulreaper_Cmode.mdl",
	{
		{
			0,
			"sprites/ef_soulreaper_p.spr",
			30,
			{
				{ 0, false, true, 1.0, 0, 0, 0 },
			}
		},
	}
};

ConstSprList s_ModelVSoulReaper = {
	"v_soulreaper.mdl",
	{
		{
			2,
			"sprites/ef_soulreaper_green01.spr",
			30,
			{
				{ 0, true, false, 1.0, 0, 0, 0 },
				{ 1, true, false, 1.0, 0, 0, 0 },
				{ 2, true, false, 1.0, 0, 0, 0 },
				{ 3, true, false, 1.0, 0, 0, 0 },
				{ 4, true, false, 1.0, 0, 0, 0 },
				{ 9, true, false, 1.0, 0, 0, 0 },
			}
		},
		{
			3,
			"sprites/ef_soulreaper_green02.spr",
			30,
			{
				{ 0, true, false, 1.0, 0, 0, 0 },
				{ 1, true, false, 1.0, 0, 0, 0 },
				{ 2, true, false, 1.0, 0, 0, 0 },
				{ 3, true, false, 1.0, 0, 0, 0 },
				{ 4, true, false, 1.0, 0, 0, 0 },
				{ 9, true, false, 1.0, 0, 0, 0 },
			}
		},
		{
			5,
			"sprites/ef_soulreaper_sphere_c.spr",
			30,
			{
				{ 5, false, false, 1.0, 45, 88, 0 },
				{ 6, false, false, 1.0, 45, 88, 0 },
				{ 7, false, false, 1.0, 45, 88, 0 },
				{ 8, false, true, 1.0, 0, 88, 0 },
			}
		},
		{
			8,
			"sprites/ef_soulreaper_blue01.spr",
			30,
			{
				{ 5, true, false, 1.0, 0, 0, 0 },
				{ 6, true, false, 1.0, 0, 0, 0 },
				{ 7, true, false, 1.0, 0, 0, 0 },
				{ 8, true, false, 1.0, 0, 0, 0 },
			}
		},
		{
			9,
			"sprites/ef_soulreaper_blue02.spr",
			30,
			{
				{ 5, true, false, 1.0, 0, 0, 0 },
				{ 6, true, false, 1.0, 0, 0, 0 },
				{ 7, true, false, 1.0, 0, 0, 0 },
				{ 8, true, false, 1.0, 0, 0, 0 },
			}
		},
		{
			10,
			"sprites/ef_soulreaper_cmode.spr",
			30,
			{
				{ 5, true, false, 1.0, 0, 0, 0 },
				{ 6, true, false, 1.0, 0, 0, 0 },
				{ 7, true, false, 1.0, 0, 0, 0 },
				{ 8, true, false, 1.0, 0, 0, 0 },
			}
		},
		{
			11,
			"sprites/ef_soulreaper_amode.spr",
			30,
			{
				{ 0, true, false, 1.0, 0, 0, 0 },
				{ 1, true, false, 1.0, 0, 0, 0 },
				{ 2, true, false, 1.0, 0, 0, 0 },
				{ 3, true, false, 1.0, 0, 0, 0 },
				{ 4, true, false, 1.0, 0, 0, 0 },
				{ 9, true, false, 1.0, 0, 0, 0 },
			}
		},
	}
};

ConstSprList s_ModelVChainMG = {
	"v_chainmg.mdl",
	{
		{
			0,
			"sprites/ef_chainmg_v01.spr",
			30,
			{
				{ 3, false, true, 1.0, 0, 0, 4 },
				{ 4, false, true, 1.0, 0, 5, 9 },
			}
		},
	}
};
// ConstSprList s_ModelM82A1AngelicBeast = {
// 	"PV-M82A1_AngelicBeast.mdl",
// 	"FXBASE.bmp",
// 	"sprites/v_m82a1angelicbeast.spr",
// 	30.0,
// 	{
// 		{ 0, true, false, 1.0, 0, 0, 0 },
// 		{ 1, true, false, 1.0, 0, 0, 0 },
// 		{ 2, true, false, 1.0, 0, 0, 0 },
// 		{ 3, true, false, 1.0, 0, 0, 0 },
// 		{ 4, true, false, 1.0, 0, 0, 0 },
// 		{ 5, true, false, 1.0, 0, 0, 0 },
// 		{ 6, true, false, 1.0, 0, 0, 0 },
// 		{ 7, true, false, 1.0, 0, 0, 0 },
// 		{ 8, true, false, 1.0, 0, 0, 0 },
// 		{ 9, true, false, 1.0, 0, 0, 0 },
// 		{ 10, true, false, 1.0, 0, 0, 0 },
// 		{ 11, true, false, 1.0, 0, 0, 0 },
// 	}
// };


std::vector<SprList> g_vecBindSprList;

static CBindSprManager g_SprBindManager;

CBindSprManager& BindSprManager(void)
{
	return g_SprBindManager;
}

void CBindSprManager::Vidinit(void)
{
	g_vecBindSprList.clear();

	LoadSpr(&s_ModelEFDvHammer2);
	LoadSpr(&s_ModelEFIgniteBomb);
	LoadSpr(&s_ModelEFLeapStrikeGun_Fall);
	LoadSpr(&s_ModelEFLeapStrikeGunFlying);
	LoadSpr(&s_ModelEFLeapStrikeGun_JumpDash);
	LoadSpr(&s_ModelEFLeapStrikeGunJump_FX);
	LoadSpr(&s_ModelEFLeapStrikeGun_Landing);
	LoadSpr(&s_ModelEFLinkAttackSG_Field);
	LoadSpr(&s_ModelEFM3DragonEX);
	LoadSpr(&s_ModelEFM3DragonEX_Sign);
	LoadSpr(&s_ModelLeapStrikeGun);
	LoadSpr(&s_ModelLeapStrikeGunPart);	
	LoadSpr(&s_ModelLinkAttackSG);
	LoadSpr(&s_ModelLinkAttackSGPart);
	LoadSpr(&s_ModelY23S3BuffM249);
	LoadSpr(&s_ModelBuffClass23S3CT);
	LoadSpr(&s_ModelSoulReaperShootC02);
	LoadSpr(&s_ModelSoulReaperShootC);
	LoadSpr(&s_ModelSoulReaperSkill);
	LoadSpr(&s_ModelPSoulReaper);
	LoadSpr(&s_ModelSoulReaperAmode);
	LoadSpr(&s_ModelSoulReaperCmode);
	LoadSpr(&s_ModelVSoulReaper);
	LoadSpr(&s_ModelVChainMG);
	//LoadSpr(&s_ModelM82A1AngelicBeast);
}

int CBindSprManager::LoadSpr(ConstSprList *List)
{
	if (List == nullptr)
		return 0;

	g_vecBindSprList.emplace_back();
	SprList& Temp = g_vecBindSprList.back();

	sprintf(Temp.mdlname, "%s", List->mdlname);
	Temp.vecBindSpr = List->vecBindSpr;

	return 1;
}
