#include "extdll.h"
#include "util.h"
#include "cbase.h"

#include "monster_entity.h"

#include "monster_manager.h"

#include <vector>
#include <algorithm>

namespace sv {

class CMonsterManager::impl_t
{
public:
	std::vector<std::weak_ptr<CMonster *>> m_vecEntities;
	size_t m_iMaxEntitiesNum;
	float m_flAutoGcRatio;

	void ResizeEntities(size_t new_size)
	{
		if (m_vecEntities.size() <= new_size)
			return;

		std::vector<std::shared_ptr<CMonster *>> vecAvailable;
		std::transform(m_vecEntities.begin(), m_vecEntities.end(), std::back_inserter(vecAvailable),
				[](const std::weak_ptr<CMonster *> &p) { return p.lock(); });
		vecAvailable.erase(std::remove(vecAvailable.begin(), vecAvailable.end(), nullptr), vecAvailable.end());

		std::nth_element(vecAvailable.begin(), vecAvailable.begin() + new_size, vecAvailable.end(),
				[](const std::shared_ptr<CMonster *> &a, const std::shared_ptr<CMonster *> &b) { return (*a)->m_flTimeLastActive > (*b)->m_flTimeLastActive; });

		std::for_each(vecAvailable.begin() + new_size, vecAvailable.end(),
				[](const std::shared_ptr<CMonster *> &e) {(*e)->Killed(nullptr, GIB_NORMAL);});

		m_vecEntities.assign(vecAvailable.begin(), vecAvailable.end());
	}

	void AutoResizeCheck()
	{
		if (m_vecEntities.size() >= m_iMaxEntitiesNum) {
			// kills a quarter inactive zombies
			ResizeEntities(m_iMaxEntitiesNum * m_flAutoGcRatio);
		}
	}
};

CMonsterManager::CMonsterManager() : pimpl(new impl_t())
{
	SetMaxNumOfEntity(256);
	SetAutoGcRatio(0.8f);
}

CMonsterManager::~CMonsterManager() // frees the pimpl
{

}

void CMonsterManager::OnEntityAdd(std::shared_ptr<CMonster *> &holder)
{
	assert(holder != nullptr);
	pimpl->AutoResizeCheck();
	pimpl->m_vecEntities.push_back(holder);
}

size_t CMonsterManager::EntityCount()
{
	return pimpl->m_vecEntities.size();
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

}
