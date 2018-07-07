#ifndef KNIFE_DEFAULT_H
#define KNIFE_DEFAULT_H
#ifdef _WIN32
#pragma once
#endif

#include "knife.h"

class CKnifeHelper_Default : IKnifeHelper
{
public:
	virtual void Deploy();
	virtual void Spawn();
};

#endif