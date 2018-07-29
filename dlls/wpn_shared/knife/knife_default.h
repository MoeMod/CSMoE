#ifndef KNIFE_DEFAULT_H
#define KNIFE_DEFAULT_H
#ifdef _WIN32
#pragma once
#endif

#include "knife.h"

class CKnifeHelper_Default : public IKnifeHelper
{
public:
	BOOL Deploy(CKnife *pKnife) override;
	void Holster(CKnife *pKnife, int skiplocal) override;

	void PrimaryAttack(CKnife *pKnife) override { Swing(pKnife, TRUE); }
	void SecondaryAttack(CKnife *pKnife) override { Stab(pKnife, TRUE); }
	void Spawn(CKnife *pKnife);

private:
	int Swing(CKnife *pKnife, int fFirst);
	int Stab(CKnife *pKnife, int fFirst);
};

#endif