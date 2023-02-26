/*
*
*   This program is free software; you can redistribute it and/or modify it
*   under the terms of the GNU General Public License as published by the
*   Free Software Foundation; either version 2 of the License, or (at
*   your option) any later version.
*
*   This program is distributed in the hope that it will be useful, but
*   WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*   General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software Foundation,
*   Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*   In addition, as a special exception, the author gives permission to
*   link the code of this program with the Half-Life Game Engine ("HL
*   Engine") and Modified Game Libraries ("MODs") developed by Valve,
*   L.L.C ("Valve").  You must obey the GNU General Public License in all
*   respects for all of the code used other than the HL Engine and MODs
*   from Valve.  If you modify this file, you may extend this exception
*   to your version of the file, but you are not obligated to do so.  If
*   you do not wish to do so, delete this exception statement from your
*   version.
*
*/

#ifndef AMMO_H
#define AMMO_H
#ifdef _WIN32
#pragma once
#endif

namespace sv {

class C9MMAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity *pOther);
};

class CBuckShotAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity *pOther);
};

class C556NatoAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity *pOther);
};

class C556NatoBoxAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity *pOther);
};

class C762NatoAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity *pOther);
};

class C45ACPAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity *pOther);
};

class C50AEAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity *pOther);
};

class C338MagnumAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity *pOther);
};

class C57MMAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity *pOther);
};

class C357SIGAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity *pOther);
};

class C46MMAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity *pOther);
};

class C50BMGAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity *pOther);
};

class CCannonAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity *pOther);
};

class CZeusAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CGungnirAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity *pOther);
};

class CSgdrillAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity *pOther);
};

class C12GaugeAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity *pOther);
};

class CChainsawOil : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity *pOther);
};

class CBowArrowAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity *pOther);
};

class C44Magnum : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity *pOther);
};

class CVoidpistolAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CDgaxeAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class C50BMGHolyAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class C762NatoBoxAMEAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};


class C556AZAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CBuckBcsAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class C762NatoBoxAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class C556NatoStarAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class C50AZAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CEnergyCellsAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};
class CEnergyBatteryAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CSkullT9Ammo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CRailBeamAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};


class CSVDEXGrenadeAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CPlasmaAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CReviveGunAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CM32VenomGrenade : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};


class CCannonEXAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CSpearAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class C556LVBAmmo : public CBasePlayerAmmo
{
public:
	void Spawn(void);
	void Precache(void);
	BOOL AddAmmo(CBaseEntity* pOther);
};

class CSGMissileBcsAmmo : public CBasePlayerAmmo
{
public:
	void Spawn(void);
	void Precache(void);
	BOOL AddAmmo(CBaseEntity* pOther);
};

class CReviveAmmo : public CBasePlayerAmmo
{
public:
	void Spawn(void);
	void Precache(void);
	BOOL AddAmmo(CBaseEntity* pOther);
};

class C408CheyTacAmmo : public CBasePlayerAmmo
{
public:
	void Spawn(void);
	void Precache(void);
	BOOL AddAmmo(CBaseEntity* pOther);
};

class C792NatoAmmo : public CBasePlayerAmmo
{
public:
	void Spawn(void);
	void Precache(void);
	BOOL AddAmmo(CBaseEntity* pOther);
};

class CBuffBulletAmmo : public CBasePlayerAmmo
{
public:
	void Spawn(void);
	void Precache(void);
	BOOL AddAmmo(CBaseEntity* pOther);
};

class C50AEAZAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CDepletedAlloyAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CBuckshotAZAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CEtherAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CWingGunAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CLaserSGAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CWonderCannonAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CEnergyCellAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CPianoGunEnergyCellAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CBuckshotKAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CCrossBowEX21Ammo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CAmethystAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CBoltsAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class C50AEAHEAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class C50AEHolyAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CCoilAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class C939MMAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CAirGasAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CPoisonGunAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CNitrogenGasAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CLiquefiedGasAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CLockOnGunAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CBloodHunterAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CBuffAugBulletAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CMGSMAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CThunderPistolAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class C556AHEAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class C556KAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class C545NatoAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class C762KBoxAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class C57MMKAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class C45ACPKAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CLaserFistAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CAmmoGuillotine : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CAmmoGuillotineex : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CSpearmAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class C20MMAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CBlockBulletAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CBlockMissileAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CBlockMGMissileAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};


class CBlockBuckShotAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};


class CBlockASMissileAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CCoilBoltsAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CBouncerAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CBStarChaserSRAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CMusketAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CTurbulent1Ammo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CBuffNG7Ammo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CBuffFiveSevenAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CRestrictionPistolAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CWaterPistolAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CTKnifeAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CLaserMiniGunAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CDrillGunAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class C45ACPAZAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};
class CMagicSgAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};


class C14MMAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class C9MMBoxAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class C40MMGrenadeAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class C40MMGrenade2Ammo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class C84MMRocketAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class C84MMRocket2Ammo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CHaloGunAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};

class CDartAmmo : public CBasePlayerAmmo
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual BOOL AddAmmo(CBaseEntity* pOther);
};
}

#endif // AMMO_H
