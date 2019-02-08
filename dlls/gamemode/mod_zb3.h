//
// Created by 小白白 on 2019-02-09.
//

#ifndef PROJECT_MOD_ZB3_H
#define PROJECT_MOD_ZB3_H

#include "mod_zb2.h"

class CMod_ZombieHero : public CMod_ZombieMod2
{
public:
	void InstallPlayerModStrategy(CBasePlayer *) override;

protected:
	void PickZombieOrigin() override;


protected:
	virtual void PickHero();
	virtual size_t HeroNum();
	virtual void MakeHero(CBasePlayer *);
};

class CPlayerModStrategy_ZB3 : public CPlayerModStrategy_ZB2
{
public:
	CPlayerModStrategy_ZB3(CBasePlayer *player, CMod_ZombieHero *mp);

	bool CanUseZombieSkill() override;

private:
	CMod_ZombieHero * const m_pModZB3;
};


#endif //PROJECT_MOD_ZB3_H
