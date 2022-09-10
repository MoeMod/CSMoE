#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "gamerules.h"
#include "weapons.h"
#include "weapons_precache.h"

#include "cbase/cbase_memory.h"


// called by worldspawn
namespace sv {

template<class T>
void W_PrecacheOther()
{
	auto pEntity = CreateClassPtr<T>();
	if (!pEntity)
	{
		ALERT(at_console, "NULL Ent (%s) in UTIL_PrecacheOther\n", typeid(T).name());
		return;
	}

	pEntity->Precache();

	REMOVE_ENTITY(pEntity->edict());
}

template<class T>
void PrecacheOtherWeapon()
{
	auto pEntity = CreateClassPtr<T>();
	if (!pEntity)
	{
		ALERT(at_console, "NULL Ent (%s) in UTIL_PrecacheOther\n", typeid(T).name());
		return;
	}

	pEntity->Precache();

	if (pEntity != NULL)
	{
		ItemInfo II;
		Q_memset(&II, 0, sizeof(II));

		pEntity->Precache();
		if (((CBasePlayerItem *)pEntity)->GetItemInfo(&II))
		{
			CBasePlayerItem::ItemInfoArray[ II.iId ] = II;

			if (II.pszAmmo1 != NULL && *II.pszAmmo1 != '\0')
			{
				AddAmmoNameToAmmoRegistry(II.pszAmmo1);
			}

			if (II.pszAmmo2 != NULL && *II.pszAmmo2 != '\0')
			{
				AddAmmoNameToAmmoRegistry(II.pszAmmo2);
			}
		}
	}

	REMOVE_ENTITY(pEntity->edict());
}

void W_Precache()
{
	CBasePlayerItem::ItemInfoArray.clear();
	//Q_memset(CBasePlayerItem::ItemInfoArray, 0, sizeof(CBasePlayerItem::ItemInfoArray));
	Q_memset(CBasePlayerItem::AmmoInfoArray, 0, sizeof(CBasePlayerItem::AmmoInfoArray));
	/*Q_memset(CBasePlayerItem::ItemInfoArray, 0, ARRAYSIZE(CBasePlayerItem::ItemInfoArray));
	Q_memset(CBasePlayerItem::AmmoInfoArray, 0, ARRAYSIZE(CBasePlayerItem::AmmoInfoArray));*/
	giAmmoIndex = 0;

	// custom items...

	// common world objects
	UTIL_PrecacheOther("item_suit");
	UTIL_PrecacheOther("item_battery");
	UTIL_PrecacheOther("item_antidote");
	UTIL_PrecacheOther("item_security");
	UTIL_PrecacheOther("item_longjump");
	UTIL_PrecacheOther("item_kevlar");
	UTIL_PrecacheOther("item_assaultsuit");
	UTIL_PrecacheOther("item_thighpack");

	// awp magnum
	UTIL_PrecacheOtherWeapon("weapon_awp");
	UTIL_PrecacheOther("ammo_338magnum");

	UTIL_PrecacheOtherWeapon("weapon_g3sg1");
	UTIL_PrecacheOtherWeapon("weapon_ak47");
	UTIL_PrecacheOtherWeapon("weapon_scout");
	UTIL_PrecacheOther("ammo_762nato");

	// m249
	UTIL_PrecacheOtherWeapon("weapon_m249");
	UTIL_PrecacheOther("ammo_556natobox");

	UTIL_PrecacheOtherWeapon("weapon_m4a1");
	UTIL_PrecacheOtherWeapon("weapon_sg552");
	UTIL_PrecacheOtherWeapon("weapon_aug");
	UTIL_PrecacheOtherWeapon("weapon_sg550");
	UTIL_PrecacheOther("ammo_556nato");

	// shotgun
	UTIL_PrecacheOtherWeapon("weapon_m3");
	UTIL_PrecacheOtherWeapon("weapon_xm1014");
	UTIL_PrecacheOther("ammo_buckshot");

	UTIL_PrecacheOtherWeapon("weapon_usp");
	UTIL_PrecacheOtherWeapon("weapon_mac10");
	UTIL_PrecacheOtherWeapon("weapon_ump45");
	UTIL_PrecacheOther("ammo_45acp");

	UTIL_PrecacheOtherWeapon("weapon_fiveseven");
	UTIL_PrecacheOtherWeapon("weapon_p90");
	UTIL_PrecacheOther("ammo_57mm");

	// deagle
	UTIL_PrecacheOtherWeapon("weapon_deagle");
	UTIL_PrecacheOther("ammo_50ae");

	// p228
	UTIL_PrecacheOtherWeapon("weapon_p228");
	UTIL_PrecacheOther("ammo_357sig");

	// knife
	UTIL_PrecacheOtherWeapon("weapon_knife");

	UTIL_PrecacheOtherWeapon("weapon_glock18");
	UTIL_PrecacheOtherWeapon("weapon_mp5navy");
	UTIL_PrecacheOtherWeapon("weapon_tmp");
	UTIL_PrecacheOtherWeapon("weapon_elite");
	UTIL_PrecacheOther("ammo_9mm");

	UTIL_PrecacheOtherWeapon("weapon_flashbang");
	UTIL_PrecacheOtherWeapon("weapon_hegrenade");
	UTIL_PrecacheOtherWeapon("weapon_smokegrenade");
	UTIL_PrecacheOtherWeapon("weapon_c4");
	UTIL_PrecacheOtherWeapon("weapon_galil");
	UTIL_PrecacheOtherWeapon("weapon_famas");


	UTIL_PrecacheOtherWeapon("weapon_mp7a1d");
	UTIL_PrecacheOther("ammo_46mm");
	UTIL_PrecacheOtherWeapon("weapon_ak47l");
	UTIL_PrecacheOtherWeapon("weapon_deagled");
	UTIL_PrecacheOtherWeapon("weapon_wa2000");
	UTIL_PrecacheOtherWeapon("weapon_m95");
	UTIL_PrecacheOtherWeapon("weapon_buffsg552ex");
	UTIL_PrecacheOtherWeapon("weapon_as50");
	UTIL_PrecacheOtherWeapon("weapon_bendita");
	UTIL_PrecacheOther("ammo_50bmgholy");
	UTIL_PrecacheOtherWeapon("weapon_skull5");
	UTIL_PrecacheOtherWeapon("z4b_k1ases");
	UTIL_PrecacheOtherWeapon("z4b_m1887se");
	UTIL_PrecacheOtherWeapon("weapon_m4a1dragon");
	UTIL_PrecacheOtherWeapon("z4b_m37se");
	UTIL_PrecacheOtherWeapon("weapon_ak47dragon");
	UTIL_PrecacheOtherWeapon("weapon_stg44");
	UTIL_PrecacheOther("ammo_792nato");
	UTIL_PrecacheOtherWeapon("weapon_gatlingex");
	UTIL_PrecacheOtherWeapon("knife_knifedragon");
	UTIL_PrecacheOther("ammo_50bmg");
	UTIL_PrecacheOtherWeapon("weapon_kriss");
	UTIL_PrecacheOtherWeapon("weapon_thompson");
	UTIL_PrecacheOtherWeapon("weapon_m1887");
	UTIL_PrecacheOtherWeapon("weapon_tar21");
	UTIL_PrecacheOtherWeapon("weapon_xm8c");
	UTIL_PrecacheOtherWeapon("weapon_xm8s");
	UTIL_PrecacheOtherWeapon("weapon_scarl");
	UTIL_PrecacheOtherWeapon("weapon_scarh");
	UTIL_PrecacheOtherWeapon("weapon_m14ebr");
	UTIL_PrecacheOtherWeapon("weapon_balrog7");
	UTIL_PrecacheOtherWeapon("weapon_anaconda");
	UTIL_PrecacheOtherWeapon("weapon_m134");
	UTIL_PrecacheOtherWeapon("weapon_m134ex");
	UTIL_PrecacheOther("ammo_762natobox");
	UTIL_PrecacheOtherWeapon("weapon_k1a");
	UTIL_PrecacheOtherWeapon("weapon_m134xmas");
	UTIL_PrecacheOtherWeapon("weapon_m95xmas");
	UTIL_PrecacheOtherWeapon("weapon_mg3xmas");
	UTIL_PrecacheOtherWeapon("weapon_janus7xmas");
	UTIL_PrecacheOtherWeapon("weapon_chainsaw");
	UTIL_PrecacheOther("ammo_chainsaw");
	UTIL_PrecacheOtherWeapon("weapon_desperado");
	UTIL_PrecacheOther("ammo_44magnum");
	UTIL_PrecacheOtherWeapon("weapon_tbarrel");
	UTIL_PrecacheOtherWeapon("weapon_sgdrill");
	UTIL_PrecacheOther("ammo_sgdrill_buckshot");
	UTIL_PrecacheOtherWeapon("weapon_mg3");
	UTIL_PrecacheOtherWeapon("weapon_mg36");
	UTIL_PrecacheOtherWeapon("csgo_mag7");
	UTIL_PrecacheOtherWeapon("csgo_negev");
	UTIL_PrecacheOtherWeapon("csgo_sawedoff");
	UTIL_PrecacheOtherWeapon("csgo_bizon");
	UTIL_PrecacheOtherWeapon("csgo_tec9");
	UTIL_PrecacheOtherWeapon("csgo_zeus");
	UTIL_PrecacheOtherWeapon("csgo_cz75");
	UTIL_PrecacheOtherWeapon("csgo_r8");
	UTIL_PrecacheOtherWeapon("weapon_svdex");
	UTIL_PrecacheOtherWeapon("weapon_gatling");
	UTIL_PrecacheOtherWeapon("weapon_bow");
	UTIL_PrecacheOtherWeapon("weapon_huntbow");
	UTIL_PrecacheOtherWeapon("weapon_voidpistol");
	UTIL_PrecacheOther("ammo_voidpistol");
	UTIL_PrecacheOtherWeapon("weapon_starchaserar");
	UTIL_PrecacheOther("ammo_556natostar");
	UTIL_PrecacheOtherWeapon("weapon_m2");
	UTIL_PrecacheOtherWeapon("weapon_cannon");
	UTIL_PrecacheOther("ammo_cannon");
	UTIL_PrecacheOther("ammo_bow");
	UTIL_PrecacheOtherWeapon("weapon_gungnir");
	UTIL_PrecacheOther("ammo_gungnir");
	UTIL_PrecacheOtherWeapon("knife_skullaxe");
	UTIL_PrecacheOtherWeapon("knife_katana");
	UTIL_PrecacheOtherWeapon("knife_dragonsword");
	UTIL_PrecacheOtherWeapon("knife_dgaxe");
	UTIL_PrecacheOther("ammo_dgaxe");
	UTIL_PrecacheOtherWeapon("weapon_infinity");
	UTIL_PrecacheOtherWeapon("weapon_infinityss");
	UTIL_PrecacheOtherWeapon("weapon_infinitysb");
	UTIL_PrecacheOtherWeapon("weapon_infinitysr");
	UTIL_PrecacheOtherWeapon("weapon_mp7a1c");
	UTIL_PrecacheOtherWeapon("weapon_mp7a1p");
	UTIL_PrecacheOtherWeapon("knife_zombi");
	UTIL_PrecacheOtherWeapon("weapon_zombibomb");
	UTIL_PrecacheOtherWeapon("weapon_zombibomb2");
	UTIL_PrecacheOtherWeapon("weapon_infinityex1");
	UTIL_PrecacheOtherWeapon("weapon_infinityex2");
	UTIL_PrecacheOtherWeapon("z4b_infinityx");
	UTIL_PrecacheOtherWeapon("z4b_m134heroi");
	UTIL_PrecacheOtherWeapon("z4b_deagleb");
	UTIL_PrecacheOtherWeapon("z4b_xm2010pc");
	UTIL_PrecacheOtherWeapon("z4b_m4a1razer");
	UTIL_PrecacheOtherWeapon("z4b_awpnvidia");
	UTIL_PrecacheOtherWeapon("z4b_dmp7a1x");
	UTIL_PrecacheOtherWeapon("z4b_m4a1x");
	UTIL_PrecacheOtherWeapon("z4b_m4a1mw");
	UTIL_PrecacheOtherWeapon("z4b_ak47x");
	UTIL_PrecacheOtherWeapon("z4b_cannontiger");
	UTIL_PrecacheOtherWeapon("z4b_cannonplasma");
	UTIL_PrecacheOtherWeapon("z4b_frequency1");
	UTIL_PrecacheOtherWeapon("knife_dualsword");
	UTIL_PrecacheOtherWeapon("weapon_gunkata");
	UTIL_PrecacheOtherWeapon("weapon_balrog11");
	UTIL_PrecacheOtherWeapon("weapon_balrog11b");
	UTIL_PrecacheOtherWeapon("weapon_balrog11wc");
	UTIL_PrecacheOther("ammo_buckbcs");
	UTIL_PrecacheOtherWeapon("knife_lance");
	UTIL_PrecacheOtherWeapon("weapon_m249ex");
	UTIL_PrecacheOther("ammo_50az");
	UTIL_PrecacheOtherWeapon("weapon_sfsmg");
	UTIL_PrecacheOtherWeapon("weapon_sfmg");
	UTIL_PrecacheOtherWeapon("weapon_sfgun");
	UTIL_PrecacheOtherWeapon("weapon_qbarrel");
	UTIL_PrecacheOtherWeapon("weapon_buffm249");
	UTIL_PrecacheOther("ammo_762natoboxame");
	UTIL_PrecacheOtherWeapon("weapon_buffm4");
	UTIL_PrecacheOther("ammo_buffbullet");
	UTIL_PrecacheOtherWeapon("knife_stormgiant");
	UTIL_PrecacheOtherWeapon("weapon_buffak");
	UTIL_PrecacheOtherWeapon("weapon_bison");
	UTIL_PrecacheOtherWeapon("knife_nataknife");
	UTIL_PrecacheOtherWeapon("knife_thanatos9");
	UTIL_PrecacheOtherWeapon("weapon_y21s1jetgunmd");
	UTIL_PrecacheOther("ammo_energycells");
	UTIL_PrecacheOther("ammo_energybattery");
	UTIL_PrecacheOtherWeapon("knife_skullt9");
	UTIL_PrecacheOther("ammo_skullt9");
	UTIL_PrecacheOtherWeapon("z4b_barrettd");
	UTIL_PrecacheOtherWeapon("weapon_sfsniper");
	UTIL_PrecacheOther("ammo_railbeam");
	UTIL_PrecacheOther("ammo_svdexgrenade");
	UTIL_PrecacheOtherWeapon("weapon_plasmagun");
	UTIL_PrecacheOther("ammo_plasma");
	UTIL_PrecacheOtherWeapon("knife_runeblade");
	UTIL_PrecacheOtherWeapon("knife_janus9");
	UTIL_PrecacheOtherWeapon("knife_balrog9");
	UTIL_PrecacheOtherWeapon("knife_summonknife");
	UTIL_PrecacheOtherWeapon("weapon_revivegun");
	UTIL_PrecacheOther("ammo_revivegun");
	UTIL_PrecacheOtherWeapon("knife_holysword");
	UTIL_PrecacheOtherWeapon("weapon_y20s2scard");
	UTIL_PrecacheOtherWeapon("weapon_zombielaw");
	UTIL_PrecacheOtherWeapon("weapon_m32venom");
	UTIL_PrecacheOther("ammo_m32venomgrenade");
	UTIL_PrecacheOtherWeapon("weapon_m95tiger");
	UTIL_PrecacheOtherWeapon("knife_y22s1holyswordmb");
	UTIL_PrecacheOtherWeapon("knife_magicknife");
	UTIL_PrecacheOtherWeapon("knife_whipsword");
	UTIL_PrecacheOtherWeapon("knife_zombi_z5");
	UTIL_PrecacheOtherWeapon("z4b_dualkukri");
	UTIL_PrecacheOtherWeapon("weapon_cannonex");
	UTIL_PrecacheOther("cannonex_dragon");
	UTIL_PrecacheOtherWeapon("weapon_speargun");
	UTIL_PrecacheOtherWeapon("z4b_dualslayer");
	UTIL_PrecacheOtherWeapon("weapon_hk23");
	UTIL_PrecacheOtherWeapon("weapon_m60");
	UTIL_PrecacheOtherWeapon("knife_jknife");
	UTIL_PrecacheOtherWeapon("knife_combat");
	UTIL_PrecacheOtherWeapon("knife_mastercombat");	
	UTIL_PrecacheOtherWeapon("knife_hdagger");
	UTIL_PrecacheOtherWeapon("knife_axe");
	UTIL_PrecacheOtherWeapon("knife_hammer");
	UTIL_PrecacheOtherWeapon("weapon_mk48");
	UTIL_PrecacheOtherWeapon("weapon_lightzg");
	UTIL_PrecacheOtherWeapon("weapon_heavyzg");
	UTIL_PrecacheOtherWeapon("weapon_bpgm");
	UTIL_PrecacheOtherWeapon("weapon_m950");
	UTIL_PrecacheOtherWeapon("weapon_tmpdragon");
	UTIL_PrecacheOtherWeapon("weapon_zgun");
	UTIL_PrecacheOtherWeapon("weapon_k3");
	UTIL_PrecacheOtherWeapon("weapon_as50g");
	UTIL_PrecacheOtherWeapon("weapon_ak47g");
	UTIL_PrecacheOtherWeapon("weapon_sgmissile");
	UTIL_PrecacheOther("ammo_sgmissilebcs");
	UTIL_PrecacheOther("ammo_revive");
	UTIL_PrecacheOtherWeapon("weapon_sgmissilem");
	UTIL_PrecacheOtherWeapon("weapon_sgmissileex");
	UTIL_PrecacheOtherWeapon("weapon_cheytaclrrs");
	UTIL_PrecacheOther("ammo_408cheytac");
	UTIL_PrecacheOtherWeapon("weapon_m400");
	UTIL_PrecacheOtherWeapon("weapon_m82");
	UTIL_PrecacheOtherWeapon("weapon_m24");
	UTIL_PrecacheOtherWeapon("weapon_trg42");
	UTIL_PrecacheOtherWeapon("weapon_trg42g");
	UTIL_PrecacheOtherWeapon("weapon_aw50");
	UTIL_PrecacheOtherWeapon("weapon_sl8");
	UTIL_PrecacheOtherWeapon("weapon_sl8g");
	UTIL_PrecacheOtherWeapon("weapon_sl8ex");
	UTIL_PrecacheOtherWeapon("weapon_sprifle");
	UTIL_PrecacheOtherWeapon("weapon_spsmg");
	UTIL_PrecacheOtherWeapon("z4b_holyfist");
	UTIL_PrecacheOtherWeapon("z4b_dualstinger");
	UTIL_PrecacheOtherWeapon("z4b_freedom");
	UTIL_PrecacheOtherWeapon("z4b_csg12ss");
	UTIL_PrecacheOtherWeapon("weapon_skull8");
	UTIL_PrecacheOtherWeapon("weapon_skull4");
	UTIL_PrecacheOtherWeapon("weapon_skull6");
	UTIL_PrecacheOtherWeapon("weapon_m249ep");
	UTIL_PrecacheOtherWeapon("z4b_heartstimulater");
	UTIL_PrecacheOtherWeapon("z4b_syringe");
	UTIL_PrecacheOtherWeapon("weapon_skull1");
	UTIL_PrecacheOther("ammo_50aeaz");
	UTIL_PrecacheOther("ammo_depletedalloy");
	UTIL_PrecacheOther("ammo_buckshotaz");
	UTIL_PrecacheOtherWeapon("weapon_m3dragon");
	UTIL_PrecacheOtherWeapon("weapon_m3dragonm");
	UTIL_PrecacheOtherWeapon("weapon_herochainsaw");
	UTIL_PrecacheOtherWeapon("weapon_chainsawm");
	UTIL_PrecacheOtherWeapon("weapon_ethereal");
	UTIL_PrecacheOther("ammo_ether");
	UTIL_PrecacheOtherWeapon("weapon_railgun");
	UTIL_PrecacheOtherWeapon("weapon_thanatos7");
	UTIL_PrecacheOtherWeapon("weapon_crow7");
	UTIL_PrecacheOtherWeapon("weapon_winggun");
	UTIL_PrecacheOther("ammo_winggun");
	UTIL_PrecacheOtherWeapon("weapon_m950se");
	UTIL_PrecacheOtherWeapon("weapon_chainsawex");
	UTIL_PrecacheOtherWeapon("weapon_lasersg");
	UTIL_PrecacheOtherWeapon("weapon_wondercannon");
	UTIL_PrecacheOtherWeapon("knife_y22s2sfsword");
	UTIL_PrecacheOtherWeapon("weapon_sfpistol");
	UTIL_PrecacheOtherWeapon("weapon_y22s2sfpistol");
	UTIL_PrecacheOtherWeapon("weapon_pianogun");
	UTIL_PrecacheOtherWeapon("weapon_pianogunex");
	UTIL_PrecacheOtherWeapon("weapon_stunrifle");
	UTIL_PrecacheOtherWeapon("z4b_ragingbull");
	UTIL_PrecacheOtherWeapon("weapon_kronos12");
	UTIL_PrecacheOtherWeapon("weapon_crossbowex21");
	UTIL_PrecacheOtherWeapon("weapon_buffawp");
	UTIL_PrecacheOtherWeapon("weapon_divinetitan");
	UTIL_PrecacheOtherWeapon("weapon_railcannon");
	UTIL_PrecacheOtherWeapon("weapon_skull11");
	UTIL_PrecacheOtherWeapon("weapon_janus11");
	UTIL_PrecacheOtherWeapon("weapon_crow5");
	UTIL_PrecacheOtherWeapon("weapon_janusmk5");
	UTIL_PrecacheOtherWeapon("weapon_janus3");
	UTIL_PrecacheOtherWeapon("weapon_crow3");
	UTIL_PrecacheOtherWeapon("weapon_crow11");
	UTIL_PrecacheOtherWeapon("z4b_m60amethyst");
	UTIL_PrecacheOtherWeapon("z4b_ar57amethyst");
	UTIL_PrecacheOtherWeapon("z4b_acramethyst");
	UTIL_PrecacheOtherWeapon("z4b_dbarrelamethyst");
	UTIL_PrecacheOtherWeapon("z4b_deagleamethyst");
	UTIL_PrecacheOtherWeapon("weapon_crossbow");
	UTIL_PrecacheOtherWeapon("weapon_broad");
	UTIL_PrecacheOtherWeapon("weapon_sgdrillex");
	UTIL_PrecacheOtherWeapon("weapon_balrog1");
	UTIL_PrecacheOtherWeapon("weapon_sapientia");
	UTIL_PrecacheOtherWeapon("weapon_balrog3");
	UTIL_PrecacheOtherWeapon("weapon_coilmg");
	UTIL_PrecacheOtherWeapon("weapon_charger7");
	UTIL_PrecacheOtherWeapon("weapon_charger5");
	UTIL_PrecacheOtherWeapon("weapon_airburster");
	UTIL_PrecacheOtherWeapon("weapon_poisongun");
	UTIL_PrecacheOtherWeapon("weapon_flamethrower");
	UTIL_PrecacheOtherWeapon("weapon_watercannon");
	UTIL_PrecacheOtherWeapon("weapon_spmg");
	UTIL_PrecacheOtherWeapon("weapon_lockongun");
	UTIL_PrecacheOtherWeapon("weapon_patroldrone");
	UTIL_PrecacheOtherWeapon("z4b_aquariusplasmagun");
	UTIL_PrecacheOtherWeapon("weapon_fglauncher");
	UTIL_PrecacheOtherWeapon("weapon_bloodhunter");
	UTIL_PrecacheOtherWeapon("weapon_buffaug");
	UTIL_PrecacheOtherWeapon("weapon_buffsg552");
	UTIL_PrecacheOtherWeapon("weapon_cameragun");
	UTIL_PrecacheOtherWeapon("weapon_m1887g");
	UTIL_PrecacheOtherWeapon("weapon_mgsm");
	UTIL_PrecacheOtherWeapon("weapon_bunkerbuster");
	UTIL_PrecacheOtherWeapon("weapon_thunderpistol");
	UTIL_PrecacheOtherWeapon("weapon_balrog5");
	UTIL_PrecacheOtherWeapon("weapon_thanatos11");
	UTIL_PrecacheOtherWeapon("weapon_m1887xmas");
	UTIL_PrecacheOtherWeapon("weapon_kronos5");
	UTIL_PrecacheOtherWeapon("weapon_an94");
	UTIL_PrecacheOtherWeapon("weapon_m16a4");
	UTIL_PrecacheOtherWeapon("weapon_kronos7");
	UTIL_PrecacheOtherWeapon("weapon_kronos3");
	UTIL_PrecacheOtherWeapon("weapon_kronos1");
	UTIL_PrecacheOtherWeapon("weapon_laserfist");
	UTIL_PrecacheOtherWeapon("weapon_laserfistex");
	UTIL_PrecacheOtherWeapon("weapon_rainbowgun");
	UTIL_PrecacheOtherWeapon("weapon_guillotine");
	UTIL_PrecacheOtherWeapon("weapon_guillotineex");

	if (g_pGameRules->IsDeathmatch())
	{
		// container for dropped deathmatch weapons
		UTIL_PrecacheOther("weaponbox");
	}

	g_sModelIndexFireball = PRECACHE_MODEL("sprites/zerogxplode.spr");	// fireball
	g_sModelIndexWExplosion = PRECACHE_MODEL("sprites/WXplo1.spr");		// underwater fireball
	g_sModelIndexSmoke = PRECACHE_MODEL("sprites/steam1.spr");		// smoke
	g_sModelIndexSmokeBeam = PRECACHE_MODEL("sprites/smoke.spr");
	g_sModelIndexBubbles = PRECACHE_MODEL("sprites/bubble.spr");		// bubbles
	g_sModelIndexBloodSpray = PRECACHE_MODEL("sprites/bloodspray.spr");	// initial blood
	g_sModelIndexBloodDrop = PRECACHE_MODEL("sprites/blood.spr");		// splattered blood

	g_sModelIndexSmokePuff = PRECACHE_MODEL("sprites/smokepuff.spr");
	g_sModelIndexFireball2 = PRECACHE_MODEL("sprites/eexplo.spr");
	g_sModelIndexFireball3 = PRECACHE_MODEL("sprites/fexplo.spr");
	g_sModelIndexFireball4 = PRECACHE_MODEL("sprites/fexplo1.spr");
	g_sModelIndexRadio = PRECACHE_MODEL("sprites/radio.spr");
	g_sModelIndexZombiebomb_exp = PRECACHE_MODEL("sprites/zombiebomb_exp.spr");
	g_sModelIndexBoomerEffect = PRECACHE_MODEL("models/ef_boomer.mdl");
	g_sModelIndexBoomerEffectEx = PRECACHE_MODEL("sprites/ef_boomer_ex.spr");
	g_sModelIndexBoomerSprite = PRECACHE_MODEL("sprites/spr_boomer.spr");
	g_sModelIndexPosionEffect = PRECACHE_MODEL("models/ef_poison03.mdl");
	g_sModelIndexGuillotineGibs = PRECACHE_MODEL("models/gibs_guilotine.mdl");

	g_sModelIndexEventBombExp = PRECACHE_MODEL("sprites/heart_bombexp.spr");
	g_sModelIndexEventBombExp2 = PRECACHE_MODEL("sprites/cakeexp.spr");
	g_sModelIndexMoonCakeExp = PRECACHE_MODEL("sprites/mooncake.spr");
	g_sModelIndexHolyWater = PRECACHE_MODEL("sprites/ef_hwater.spr");

	g_sModelIndexCTGhost = PRECACHE_MODEL("sprites/b-tele1.spr");
	g_sModelIndexTGhost = PRECACHE_MODEL("sprites/c-tele1.spr");
	g_sModelIndexC4Glow = PRECACHE_MODEL("sprites/ledglow.spr");

	g_sModelIndexLaser = PRECACHE_MODEL((char*)g_pModelNameLaser);
	g_sModelIndexLaserDot = PRECACHE_MODEL("sprites/laserdot.spr");

	// used by explosions
	PRECACHE_MODEL("models/grenade.mdl");
	PRECACHE_MODEL("sprites/explode1.spr");

	PRECACHE_SOUND("weapons/debris1.wav");		// explosion aftermaths
	PRECACHE_SOUND("weapons/debris2.wav");		// explosion aftermaths
	PRECACHE_SOUND("weapons/debris3.wav");		// explosion aftermaths

	PRECACHE_SOUND("weapons/grenade_hit1.wav");	// grenade
	PRECACHE_SOUND("weapons/grenade_hit2.wav");	// grenade
	PRECACHE_SOUND("weapons/grenade_hit3.wav");	// grenade
	PRECACHE_SOUND("zombi/zombi_bomb_exp.wav");	// grenade

	PRECACHE_SOUND("weapons/bullet_hit1.wav");	// hit by bullet
	PRECACHE_SOUND("weapons/bullet_hit2.wav");	// hit by bullet

	PRECACHE_SOUND("items/weapondrop1.wav");	// weapon falls to the ground
	PRECACHE_SOUND("weapons/generic_reload.wav");

    g_pModRunning->W_Precache();
}

}
