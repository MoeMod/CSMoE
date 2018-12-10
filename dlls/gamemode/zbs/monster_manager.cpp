#include "extdll.h"
#include "util.h"
#include "cbase.h"

#include "monster_entity.h"

#include "monster_manager.h"

#include <vector>
#include <algorithm>

class CMonsterManager::impl_t
{
public:
	std::vector<CMonster *> m_vecEntities;
	size_t m_iMaxEntitiesNum;
	float m_flAutoGcRatio;
	void ResizeEntities(size_t new_size)
	{
		if (m_vecEntities.size() <= new_size)
			return;

		std::nth_element(m_vecEntities.begin(), m_vecEntities.begin() + new_size, m_vecEntities.end(), [](CMonster *a, CMonster *b) {return a->m_flTimeLastActive > b->m_flTimeLastActive; });
		std::for_each(m_vecEntities.begin() + new_size, m_vecEntities.end(), std::bind(&CBaseEntity::Killed, std::placeholders::_1, nullptr, GIB_NORMAL));
	}

	void AutoResizeCheck()
	{
		if (m_vecEntities.size() >= m_iMaxEntitiesNum)
		{
			// kills a quarter inactive zombies
			ResizeEntities(m_iMaxEntitiesNum * m_flAutoGcRatio);
		}
	}
};

CMonsterManager::CMonsterManager() : pimpl(std::make_unique<impl_t>())
{
	SetMaxNumOfEntity(64);
	SetAutoGcRatio(0.8f);
}

CMonsterManager::~CMonsterManager() // frees the pimpl
{

}

void CMonsterManager::OnEntityAdd(CMonster *ent)
{
	pimpl->AutoResizeCheck();
	pimpl->m_vecEntities.push_back(ent);
}

void CMonsterManager::OnEntityRemove(CMonster *ent)
{
	auto &v = pimpl->m_vecEntities;
	v.erase(std::find(v.begin(), v.end(), ent), v.end());
}

size_t CMonsterManager::EntityCount()
{
	return pimpl->m_vecEntities.size();
}

void CMonsterManager::EntityForEach(std::function<void(CMonster *)> functor)
{
	auto &v = pimpl->m_vecEntities;
	std::for_each(v.begin(), v.end(), functor);
}

void CMonsterManager::SetMaxNumOfEntity(size_t what)
{
	pimpl->m_iMaxEntitiesNum = what;
}

size_t CMonsterManager::GetMaxNumOfEntity()
{
	return pimpl->m_iMaxEntitiesNum;
}

void CMonsterManager::SetAutoGcRatio(float what)
{
	assert(what >= 0.0f && what <= 1.0f);
	pimpl->m_flAutoGcRatio = what;
}