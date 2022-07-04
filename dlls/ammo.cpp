#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

#include "ammo.h"

namespace sv {

void C9MMAmmo::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void C9MMAmmo::Precache()
{
	PRECACHE_MODEL("models/w_9mmclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL C9MMAmmo::AddAmmo(CBaseEntity *pOther)
{
	if (pOther->GiveAmmo(AMMO_9MM_BUY, "9mm", MAX_AMMO_9MM) == -1) {
		return FALSE;
	}

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_9mm, C9MMAmmo);

void CBuckShotAmmo::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_shotbox.mdl");
	CBasePlayerAmmo::Spawn();
}

void CBuckShotAmmo::Precache()
{
	PRECACHE_MODEL("models/w_shotbox.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL CBuckShotAmmo::AddAmmo(CBaseEntity *pOther)
{
	if (pOther->GiveAmmo(AMMO_BUCKSHOT_BUY, "buckshot", MAX_AMMO_BUCKSHOT) == -1) {
		return FALSE;
	}

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_buckshot, CBuckShotAmmo);

void C556NatoAmmo::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void C556NatoAmmo::Precache()
{
	PRECACHE_MODEL("models/w_9mmclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL C556NatoAmmo::AddAmmo(CBaseEntity *pOther)
{
	if (pOther->GiveAmmo(AMMO_556NATO_BUY, "556Nato", MAX_AMMO_556NATO) == -1) {
		return FALSE;
	}

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_556nato, C556NatoAmmo);

void C556NatoBoxAmmo::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void C556NatoBoxAmmo::Precache()
{
	PRECACHE_MODEL("models/w_9mmclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL C556NatoBoxAmmo::AddAmmo(CBaseEntity *pOther)
{
	if (pOther->GiveAmmo(AMMO_556NATOBOX_BUY, "556NatoBox", MAX_AMMO_556NATOBOX) == -1) {
		return FALSE;
	}

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_556natobox, C556NatoBoxAmmo);

void C762NatoAmmo::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void C762NatoAmmo::Precache()
{
	PRECACHE_MODEL("models/w_9mmclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL C762NatoAmmo::AddAmmo(CBaseEntity *pOther)
{
	if (pOther->GiveAmmo(AMMO_762NATO_BUY, "762Nato", MAX_AMMO_762NATO) == -1) {
		return FALSE;
	}

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_762nato, C762NatoAmmo);

void C45ACPAmmo::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void C45ACPAmmo::Precache()
{
	PRECACHE_MODEL("models/w_9mmclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL C45ACPAmmo::AddAmmo(CBaseEntity *pOther)
{
	if (pOther->GiveAmmo(AMMO_45ACP_BUY, "45acp", MAX_AMMO_45ACP) == -1) {
		return FALSE;
	}

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_45acp, C45ACPAmmo);

void C50AEAmmo::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void C50AEAmmo::Precache()
{
	PRECACHE_MODEL("models/w_9mmclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL C50AEAmmo::AddAmmo(CBaseEntity *pOther)
{
	if (pOther->GiveAmmo(AMMO_50AE_BUY, "50AE", MAX_AMMO_50AE) == -1) {
		return FALSE;
	}

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_50ae, C50AEAmmo);

void C338MagnumAmmo::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void C338MagnumAmmo::Precache()
{
	PRECACHE_MODEL("models/w_9mmclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL C338MagnumAmmo::AddAmmo(CBaseEntity *pOther)
{
	if (pOther->GiveAmmo(AMMO_338MAG_BUY, "338Magnum", MAX_AMMO_338MAGNUM) == -1) {
		return FALSE;
	}

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_338magnum, C338MagnumAmmo);

void C57MMAmmo::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void C57MMAmmo::Precache()
{
	PRECACHE_MODEL("models/w_9mmclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL C57MMAmmo::AddAmmo(CBaseEntity *pOther)
{
	if (pOther->GiveAmmo(AMMO_57MM_BUY, "57mm", MAX_AMMO_57MM) == -1) {
		return FALSE;
	}

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_57mm, C57MMAmmo);

void C357SIGAmmo::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void C357SIGAmmo::Precache()
{
	PRECACHE_MODEL("models/w_9mmclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL C357SIGAmmo::AddAmmo(CBaseEntity *pOther)
{
	if (pOther->GiveAmmo(AMMO_357SIG_BUY, "357SIG", MAX_AMMO_357SIG) == -1) {
		return FALSE;
	}

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_357sig, C357SIGAmmo);

void C46MMAmmo::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void C46MMAmmo::Precache()
{
	PRECACHE_MODEL("models/w_9mmclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL C46MMAmmo::AddAmmo(CBaseEntity *pOther)
{
	if (pOther->GiveAmmo(AMMO_46MM_BUY, "46mm", MAX_AMMO_46MM) == -1) {
		return FALSE;
	}

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_46mm, C46MMAmmo);


void C50BMGAmmo::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void C50BMGAmmo::Precache()
{
	PRECACHE_MODEL("models/w_9mmclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL C50BMGAmmo::AddAmmo(CBaseEntity *pOther)
{
	if (pOther->GiveAmmo(AMMO_50BMG_BUY, "50bmg", MAX_AMMO_50BMG) == -1) {
		return FALSE;
	}

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_50bmg, C50BMGAmmo);

void CCannonAmmo::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void CCannonAmmo::Precache()
{
	PRECACHE_MODEL("models/w_9mmclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL CCannonAmmo::AddAmmo(CBaseEntity *pOther)
{
	if (pOther->GiveAmmo(1, "CannonAmmo", 20) == -1) {
		return FALSE;
	}

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_cannon, CCannonAmmo);

void CZeusAmmo::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void CZeusAmmo::Precache()
{
	PRECACHE_MODEL("models/w_9mmclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL CZeusAmmo::AddAmmo(CBaseEntity* pOther)
{
	if (pOther->GiveAmmo(1, "ZeusAmmo", 20) == -1) {
		return FALSE;
	}

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_zeus, CZeusAmmo);

void CGungnirAmmo::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void CGungnirAmmo::Precache()
{
	PRECACHE_MODEL("models/w_9mmclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL CGungnirAmmo::AddAmmo(CBaseEntity *pOther)
{
	if (pOther->GiveAmmo(50, "GungnirAmmo", 100) == -1) {
		return FALSE;
	}

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_gungnir, CGungnirAmmo);

void CSgdrillAmmo::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void CSgdrillAmmo::Precache()
{
	PRECACHE_MODEL("models/w_9mmclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL CSgdrillAmmo::AddAmmo(CBaseEntity *pOther)
{
	if (pOther->GiveAmmo(35, "sgdrill_buckshot", 105) == -1) {
		return FALSE;
	}

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_sgdrill_buckshot, CSgdrillAmmo);

void C12GaugeAmmo::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_shotbox.mdl");
	CBasePlayerAmmo::Spawn();
}

void C12GaugeAmmo::Precache()
{
	PRECACHE_MODEL("models/w_shotbox.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL C12GaugeAmmo::AddAmmo(CBaseEntity *pOther)
{
	if (pOther->GiveAmmo(20, "12gauge", 80) == -1) {
		return FALSE;
	}

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_12gauge, C12GaugeAmmo);

void CBowArrowAmmo::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void CBowArrowAmmo::Precache()
{
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL CBowArrowAmmo::AddAmmo(CBaseEntity *pOther)
{
	ItemInfo WeaponInfo;
	CBasePlayer* player = static_cast<CBasePlayer*>(pOther);
	if (player->m_pActiveItem != nullptr && player->m_pActiveItem->GetItemInfo(&WeaponInfo))
	{
		int iMaxammo = WeaponInfo.iMaxAmmo1;
		if (pOther->GiveAmmo(5, "bowarrow", iMaxammo) == -1) {
			return FALSE;
		}
	}

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;

}

LINK_ENTITY_TO_CLASS(ammo_bow, CBowArrowAmmo);

void CChainsawOil::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void CChainsawOil::Precache()
{
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL CChainsawOil::AddAmmo(CBaseEntity *pOther)
{
	if (pOther->GiveAmmo(50, "chainsawoil", 200) == -1) {
		return FALSE;
	}

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_chainsaw, CChainsawOil);

void C44Magnum::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void C44Magnum::Precache()
{
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL C44Magnum::AddAmmo(CBaseEntity *pOther)
{
	if (pOther->GiveAmmo(14, "44magnum", 200) == -1) {
		return FALSE;
	}

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_44magnum, C44Magnum);

void CVoidpistolAmmo::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void CVoidpistolAmmo::Precache()
{
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL CVoidpistolAmmo::AddAmmo(CBaseEntity* pOther)
{
	if (pOther->GiveAmmo(50, "voidpistolammo", 100) == -1) {
		return FALSE;
	}

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_voidpistol, CVoidpistolAmmo);

void CDgaxeAmmo::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void CDgaxeAmmo::Precache()
{
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL CDgaxeAmmo::AddAmmo(CBaseEntity* pOther)
{
	if (pOther->GiveAmmo(5, "dgaxeammo", 100) == -1) {
		return FALSE;
	}

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_dgaxe, CDgaxeAmmo);


void C50BMGHolyAmmo::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void C50BMGHolyAmmo::Precache()
{
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL C50BMGHolyAmmo::AddAmmo(CBaseEntity* pOther)
{
	if (pOther->GiveAmmo(7, "50bmgholy", 70) == -1) {
		return FALSE;
	}

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_50bmgholy, C50BMGHolyAmmo);

void C762NatoBoxAMEAmmo::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void C762NatoBoxAMEAmmo::Precache()
{
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL C762NatoBoxAMEAmmo::AddAmmo(CBaseEntity* pOther)
{
	if (pOther->GiveAmmo(30, "762natoboxame", 240) == -1) {
		return FALSE;
	}

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_762natoboxame, C762NatoBoxAMEAmmo);


void C556AZAmmo::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void C556AZAmmo::Precache()
{
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL C556AZAmmo::AddAmmo(CBaseEntity* pOther)
{
	if (pOther->GiveAmmo(30, "556az", 240) == -1) {
		return FALSE;
	}

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_556az, C556AZAmmo);

void CBuckBcsAmmo::Spawn(void)
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void CBuckBcsAmmo::Precache(void)
{
	PRECACHE_MODEL("models/w_9mmclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL CBuckBcsAmmo::AddAmmo(CBaseEntity* pOther)
{
	if (pOther->GiveAmmo(0, "buckbcs", 4) == -1)
		return FALSE;

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_buckbcs, CBuckBcsAmmo);

void C762NatoBoxAmmo::Spawn(void)
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void C762NatoBoxAmmo::Precache(void)
{
	PRECACHE_MODEL("models/w_9mmclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL C762NatoBoxAmmo::AddAmmo(CBaseEntity* pOther)
{
	if (pOther->GiveAmmo(30, "762NatoBox", 240) == -1)
		return FALSE;

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_762natobox, C762NatoBoxAmmo);

void C556NatoStarAmmo::Spawn(void)
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void C556NatoStarAmmo::Precache(void)
{
	PRECACHE_MODEL("models/w_9mmclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL C556NatoStarAmmo::AddAmmo(CBaseEntity* pOther)
{
	if (pOther->GiveAmmo(30, "556NatoStar", 150) == -1)
		return FALSE;

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_556natostar, C556NatoStarAmmo);


void C50AZAmmo::Spawn(void)
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void C50AZAmmo::Precache(void)
{
	PRECACHE_MODEL("models/w_9mmclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL C50AZAmmo::AddAmmo(CBaseEntity* pOther)
{
	if (pOther->GiveAmmo(30, "50az", 200) == -1)
		return FALSE;

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_50az, C50AZAmmo);

void CEnergyCellsAmmo::Spawn(void)
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void CEnergyCellsAmmo::Precache(void)
{
	PRECACHE_MODEL("models/w_9mmclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL CEnergyCellsAmmo::AddAmmo(CBaseEntity* pOther)
{
	if (pOther->GiveAmmo(50, "EnergyCells", 200) == -1)
		return FALSE;

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_energycells, CEnergyCellsAmmo);


void CEnergyBatteryAmmo::Spawn(void)
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void CEnergyBatteryAmmo::Precache(void)
{
	PRECACHE_MODEL("models/w_9mmclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL CEnergyBatteryAmmo::AddAmmo(CBaseEntity* pOther)
{
	if (pOther->GiveAmmo(0, "EnergyBattery", 2) == -1)
		return FALSE;

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_energybattery, CEnergyBatteryAmmo);

void CSkullT9Ammo::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void CSkullT9Ammo::Precache()
{
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL CSkullT9Ammo::AddAmmo(CBaseEntity* pOther)
{
	if (pOther->GiveAmmo(5, "skullt9ammo", 100) == -1) {
		return FALSE;
	}

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_skullt9, CSkullT9Ammo);


void CRailBeamAmmo::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void CRailBeamAmmo::Precache()
{
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL CRailBeamAmmo::AddAmmo(CBaseEntity* pOther)
{
	if (pOther->GiveAmmo(1, "RailBeam", 35) == -1) {
		return FALSE;
	}

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_railbeam, CRailBeamAmmo);


void CSVDEXGrenadeAmmo::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void CSVDEXGrenadeAmmo::Precache()
{
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL CSVDEXGrenadeAmmo::AddAmmo(CBaseEntity* pOther)
{
	if (pOther->GiveAmmo(5, "svdexgrenade", 100) == -1) {
		return FALSE;
	}

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_svdexgrenade, CSVDEXGrenadeAmmo);


void CPlasmaAmmo::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void CPlasmaAmmo::Precache()
{
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL CPlasmaAmmo::AddAmmo(CBaseEntity* pOther)
{
	if (pOther->GiveAmmo(45, "plasma", 200) == -1) {
		return FALSE;
	}

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_plasma, CPlasmaAmmo);

void CReviveGunAmmo::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void CReviveGunAmmo::Precache()
{
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL CReviveGunAmmo::AddAmmo(CBaseEntity* pOther)
{
	if (pOther->GiveAmmo(60, "ReviveGunAmmo", 120) == -1) {
		return FALSE;
	}

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_revivegun, CReviveGunAmmo);

void CM32VenomGrenade::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void CM32VenomGrenade::Precache()
{
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL CM32VenomGrenade::AddAmmo(CBaseEntity* pOther)
{
	if (pOther->GiveAmmo(6, "M32VenomGrenade", 36) == -1) {
		return FALSE;
	}

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_m32venomgrenade, CM32VenomGrenade);

void CCannonEXAmmo::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void CCannonEXAmmo::Precache()
{
	PRECACHE_MODEL("models/w_9mmclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL CCannonEXAmmo::AddAmmo(CBaseEntity* pOther)
{
	if (pOther->GiveAmmo(1, "CannonEXAmmo", 30) == -1) {
		return FALSE;
	}

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_cannonex, CCannonEXAmmo);


void CSpearAmmo::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void CSpearAmmo::Precache()
{
	PRECACHE_MODEL("models/w_9mmclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL CSpearAmmo::AddAmmo(CBaseEntity* pOther)
{
	if (pOther->GiveAmmo(1, "SpearAmmo", 30) == -1) {
		return FALSE;
	}

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_spear, CSpearAmmo);

void C556LVBAmmo::Spawn(void)
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void C556LVBAmmo::Precache(void)
{
	PRECACHE_MODEL("models/w_9mmclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL C556LVBAmmo::AddAmmo(CBaseEntity* pOther)
{
	if (pOther->GiveAmmo(30, "556LVB", 90) == -1)
		return FALSE;

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_556lvb, C556LVBAmmo);

void CSGMissileBcsAmmo::Spawn(void)
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void CSGMissileBcsAmmo::Precache(void)
{
	PRECACHE_MODEL("models/w_9mmclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL CSGMissileBcsAmmo::AddAmmo(CBaseEntity* pOther)
{
	if (pOther->GiveAmmo(0, "sgmissilebcs", 10) == -1)
		return FALSE;

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_sgmissilebcs, CSGMissileBcsAmmo);

void CReviveAmmo::Spawn(void)
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void CReviveAmmo::Precache(void)
{
	PRECACHE_MODEL("models/w_9mmclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL CReviveAmmo::AddAmmo(CBaseEntity* pOther)
{
	if (pOther->GiveAmmo(0, "ReviveAmmo", 6) == -1)
		return FALSE;

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_revive, CReviveAmmo);

void C408CheyTacAmmo::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void C408CheyTacAmmo::Precache()
{
	PRECACHE_MODEL("models/w_9mmclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL C408CheyTacAmmo::AddAmmo(CBaseEntity* pOther)
{
	if (pOther->GiveAmmo(AMMO_50BMG_BUY, "408CheyTac", 30) == -1) {
		return FALSE;
	}

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_408cheytac, C408CheyTacAmmo);

void C792NatoAmmo::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void C792NatoAmmo::Precache()
{
	PRECACHE_MODEL("models/w_9mmclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL C792NatoAmmo::AddAmmo(CBaseEntity* pOther)
{
	if (pOther->GiveAmmo(AMMO_762NATO_BUY, "792Nato", MAX_AMMO_762NATO) == -1) {
		return FALSE;
	}

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_792nato, C792NatoAmmo);

void CBuffBulletAmmo::Spawn(void)
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void CBuffBulletAmmo::Precache(void)
{
	PRECACHE_MODEL("models/w_9mmclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL CBuffBulletAmmo::AddAmmo(CBaseEntity* pOther)
{
	if (pOther->GiveAmmo(10, "buffbullet", 90) == -1)
		return FALSE;

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_buffbullet, CBuffBulletAmmo);

void C50AEAZAmmo::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void C50AEAZAmmo::Precache()
{
	PRECACHE_MODEL("models/w_9mmclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL C50AEAZAmmo::AddAmmo(CBaseEntity* pOther)
{
	if (pOther->GiveAmmo(AMMO_50AE_BUY, "50AEaz", MAX_AMMO_50AE) == -1) {
		return FALSE;
	}

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_50aeaz, C50AEAZAmmo);

void CDepletedAlloyAmmo::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void CDepletedAlloyAmmo::Precache()
{
	PRECACHE_MODEL("models/w_9mmclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL CDepletedAlloyAmmo::AddAmmo(CBaseEntity* pOther)
{
	if (pOther->GiveAmmo(10, "depletedalloy", 40) == -1) {
		return FALSE;
	}

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_depletedalloy, CDepletedAlloyAmmo);

void CBuckshotAZAmmo::Spawn(void)
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_shotbox.mdl");
	CBasePlayerAmmo::Spawn();
}

void CBuckshotAZAmmo::Precache(void)
{
	PRECACHE_MODEL("models/w_shotbox.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL CBuckshotAZAmmo::AddAmmo(CBaseEntity* pOther)
{
	if (pOther->GiveAmmo(20, "sgdrill_buckshot", 40) == -1)
		return FALSE;

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_buckshotaz, CBuckshotAZAmmo);

void CEtherAmmo::Spawn(void)
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void CEtherAmmo::Precache(void)
{
	PRECACHE_MODEL("models/w_9mmclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL CEtherAmmo::AddAmmo(CBaseEntity* pOther)
{
	if (pOther->GiveAmmo(30, "ether", 90) == -1)
		return FALSE;

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_ether, CEtherAmmo);

void CWingGunAmmo::Spawn(void)
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void CWingGunAmmo::Precache(void)
{
	PRECACHE_MODEL("models/w_9mmclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL CWingGunAmmo::AddAmmo(CBaseEntity* pOther)
{
	if (pOther->GiveAmmo(50, "WingGunBuckShot", 100) == -1)
		return FALSE;

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_winggun, CWingGunAmmo);

void CLaserSGAmmo::Spawn(void)
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void CLaserSGAmmo::Precache(void)
{
	PRECACHE_MODEL("models/w_9mmclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL CLaserSGAmmo::AddAmmo(CBaseEntity* pOther)
{
	if (pOther->GiveAmmo(40, "lasersg_buckshot", 80) == -1)
		return FALSE;

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_lasersg_buckshot, CLaserSGAmmo);


void CWonderCannonAmmo::Spawn(void)
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void CWonderCannonAmmo::Precache(void)
{
	PRECACHE_MODEL("models/w_9mmclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL CWonderCannonAmmo::AddAmmo(CBaseEntity* pOther)
{
	if (pOther->GiveAmmo(30, "wondercannon_ammo", 120) == -1)
		return FALSE;

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_wondercannon, CWonderCannonAmmo);

void CEnergyCellAmmo::Spawn(void)
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void CEnergyCellAmmo::Precache(void)
{
	PRECACHE_MODEL("models/w_9mmclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL CEnergyCellAmmo::AddAmmo(CBaseEntity* pOther)
{
	if (pOther->GiveAmmo(50, "energycell", 100) == -1)
		return FALSE;

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_energycell, CEnergyCellAmmo);


void CPianoGunEnergyCellAmmo::Spawn(void)
{
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmclip.mdl");
	CBasePlayerAmmo::Spawn();
}

void CPianoGunEnergyCellAmmo::Precache(void)
{
	PRECACHE_MODEL("models/w_9mmclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");
}

BOOL CPianoGunEnergyCellAmmo::AddAmmo(CBaseEntity* pOther)
{
	if (pOther->GiveAmmo(90, "pianogun_energycell", 360) == -1)
		return FALSE;

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", VOL_NORM, ATTN_NORM);
	return TRUE;
}

LINK_ENTITY_TO_CLASS(ammo_pianogun_energycell, CPianoGunEnergyCellAmmo);
}
