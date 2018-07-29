#ifndef KNIFE_H
#define KNIFE_H
#ifdef _WIN32
#pragma once
#endif

enum KnifeID
{
	KNIFE_NORMAL = 0,
	KNIFE_NATA,
	KNIFE_HAMMER,
	KNIFE_HDAGGER,
	KNIFE_JKNIFE,
	KNIFE_AXE,
	KNIFE_COMBATKNIFE,
	KNIFE_MASTERCOMBAT,
	KNIFE_KNIFEDRAGON,
	KNIFE_KANATA = 9,
	KNIFE_SKULLAXE = 10,
	KNIFE_DRAGONTAIL = 11,
	KNIFE_HIDDENSTAFF = 12,
	KNIFE_TOMAHAWK,
	KNIFE_NATAKNIFED,
	KNIFE_KANATAD,
	KNIFE_DRAGONSWORD = 16,
	KNIFE_BALROG9 = 17,
	KNIFE_SHELTERAXE,
	KNIFE_SNAKESWORD,
	KNIFE_SFSWORD_ON = 20,
	KNIFE_SFSWORD_OFF = 21,
	KNIFE_BHDRAGGER,
	KNIFE_ZOMBI,
	KNIFE_SNOWMAN = 23,
	KNIFE_HORSEAXE,
	KNIFE_ZSH_MACHETE,
	KNIFE_ZSH_CROWBAR,
	KNIFE_ZSH_CLAWHAMMER,
	KNIFE_JANUS9 = 28,
	KNIFE_MONKEYWPNSET3 = 29,
	KNIFE_BALROG9_2 = 30,
	KNIFE_CROWBARCRAFT = 31,
	KNIFE_SPKNIFE,
	KNIFE_OZWPNSET3,
	KNIFE_TOMAHAWK_XMAS = 34,
	KNIFE_1PVE_UNARM_STANDALONEHAND,
	KNIFE_1PVE_KNIFE_STANDALONEKNIFE,
	KNIFE_SHEEPSWORD,

	KNIFE_THANATOS9 = 39,
	KNIFE_VXLKNIFE,
	KNIFE_ZBTAG = 41, // emotion_z?
	KNIFE_ZBTAGH = 42,
	KNIFE_VULCANUS9_ON = 43,
	KNIFE_VULCANUS9_OFF = 44,
	KNIFE_HZKNIFE,
	KNIFE_STORMGIANT,
	KNIFE_CROW9,
	KNIFE_RUNEBLADE = 48,
	KNIFE_IRONFAN = 49,
	KNIFE_BALROG9_3 = 50,

	KNIFE_TURBULENT9 = 52,
	KNIFE_DUALSWORD = 53,
	KNIFE_ZBS64KNIFE = 54,
	KNIFE_SKULLAXE_2,
	KNIFE_ARMTOUCH = 56,

	KNIFE_ZOMBI_TANK,
	KNIFE_ZOMBI_SPEED,
	KNIFE_ZOMBI_HEAVY,
	KNIFE_ZOMBI_PC,
	KNIFE_ZOMBI_HEAL,
	KNIFE_ZOMBI_DEIMOS,
	KNIFE_ZOMBI_DEIMOS2,
	KNIFE_ZOMBI_UNDERTAKER,
	KNIFE_ZOMBI_WITCH,
	KNIFE_ZOMBI_CHINA,
	KNIFE_ZOMBI_BOOMER,
	KNIFE_ZOMBI_RESIDENT,
	KNIFE_ZOMBI_REVIVAL,
	KNIFE_ZOMBI_TELEPORT,
	KNIFE_ZOMBI_Z4NORMAL,
	KNIFE_ZOMBI_Z4LIGHT,
	KNIFE_ZOMBI_Z4HEAVY,
	KNIFE_ZOMBI_Z4HIDE,
	KNIFE_ZOMBI_Z4HUMPBACK,
	KNIFE_SCHAND,

	KNIFE_ENDS
};

struct KnifeSoundInfo // sizeof == 84
{
	const char *szSoundDeploy;
	int iSoundSlashNum;
	const char *szSoundSlash[4];
	int iSoundHitWallNum;
	const char *szSoundHitWall[4];
	int iSoundStabNum;
	const char *szSoundStab[4];
	int iSoundHitNum;
	const char *szSoundHit[4];
};

struct KnifeModelInfo // sizeof == 28
{
	const char *v_model;
	const char *p_model;
	const char *v_model2;
	const char *p_model2;
	const char *szAnimExtension;
	float flDeployTime;
	int flDeployNextIdleTime;
};

class CKnife;

class IKnifeHelper // wtf
{
public:
	virtual BOOL Deploy(CKnife *) = 0;
	virtual BOOL CanHolster(CKnife *) { return 1; };
	virtual void Holster(CKnife *, int skiplocal) = 0;
	virtual void WeaponIdle(CKnife *) = 0;
	virtual void PrimaryAttack(CKnife *) = 0;
	virtual void SecondaryAttack(CKnife *) = 0;
	virtual void DelayPrimaryAttack(CKnife *) = 0;
	virtual void DelaySecondaryAttack(CKnife *) = 0;
	virtual BOOL UseDecrement(CKnife *) { return FALSE; }; // always return 0
	virtual void ScaleDamagePrimaryAttack(CKnife *, float in, float &out) = 0;
	virtual void ScaleDamageSecondaryAttack(CKnife *, float in, float &out) = 0;
	virtual void GetPrimaryAttackDistance(CKnife *){}; // null, called from CKnife::PrimaryAttack
	virtual void GetSecondaryAttackDistance(CKnife *){}; // null
	virtual void GetKnockBackData(CKnife *){}; // GetKnockBackData from hw??
	virtual void ph14(CKnife *){}; // null
	virtual void ph15(CKnife *){}; // null
	virtual void ph16(CKnife *){}; // null
	virtual void AddToPlayer(CKnife *){}; // 
	virtual void ItemPostFrame(CKnife *){}; // null 
	virtual void ph19(CKnife *){}; // null*/

	virtual ~IKnifeHelper() {}

};

IKnifeHelper *KnifeHelper_Get(KnifeID idx);

#endif