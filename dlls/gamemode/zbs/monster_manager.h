#ifndef MONSTER_MANAGER_H
#define MONSTER_MANAGER_H
#ifdef _WIN32
#pragma once
#endif

#include <memory>
#include <functional>

namespace sv {

class CMonsterManager
{
private:
	CMonsterManager(); // singleton creator
	friend inline CMonsterManager &MonsterManager()
	{
		static CMonsterManager x;
		return x;
	}

public:
	~CMonsterManager();

private:
	friend class CMonster;
	void OnEntityAdd(std::shared_ptr<CMonster *> &holder);

public:
	size_t EntityCount();

public: // max monster num settings
	void SetMaxNumOfEntity(size_t what);
	size_t GetMaxNumOfEntity();
	void SetAutoGcRatio(float what);

private:
	class impl_t;
	std::unique_ptr<impl_t> pimpl;

};

inline CMonsterManager &MonsterManager();

}

#endif