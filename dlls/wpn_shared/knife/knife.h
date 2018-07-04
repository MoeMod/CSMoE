#ifndef KNIFE_H
#define KNIFE_H
#ifdef _WIN32
#pragma once
#endif

struct KnifeSoundInfo
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

struct KnifeModelInfo // sizeof==28
{
	const char *v_model;
	const char *p_model;
	const char *v_model2;
	const char *p_model2;
	const char *szAnimExtension;
	float flDeployTime;
	int flDeployNextIdleTime;
};

class IBaseKnifeHelper // wtf
{
	virtual void Deploy() = 0;
	virtual void ph1() = 0;
	virtual void ph2() = 0; // init ?
	virtual void WeaponIdle() = 0;
	virtual void PrimaryAttack() = 0;
	virtual void SecondaryAttack() = 0;
	virtual void DelayPrimaryAttack() = 0;
	virtual void DelaySecondaryAttack() = 0;
	virtual void ph8() = 0; // always return 0 UseDecrement
	virtual void ScaleDamagePrimaryAttack(float in, float &out) = 0;
	virtual void ScaleDamageSecondaryAttack(float in, float &out) = 0;
	virtual void ph11() = 0; // null, get primaryattack radius?
	virtual void ph12() = 0; // null, called from CKnife::PrimaryAttack
	virtual void ph13() = 0; // GetKnockBackData from hw??
	virtual void ph14() = 0; // null
	virtual void ph15() = 0; // null
	virtual void ph16() = 0; // null
	virtual void ph17() = 0; // null
	virtual void ph18() = 0; // null 
	virtual void ph19() = 0; // null

	virtual ~IBaseKnifeHelper() {}

};

#endif