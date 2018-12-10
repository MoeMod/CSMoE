#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"

#include "bot_include.h"
#include "npc_group.h"
#include <vector>

// slow
static inline CBasePlayer *GetClosestPlayerByTravelDistance(CNavArea *startArea, TeamName team)
{
	CBasePlayer *close = NULL;
	float closeRange = 9.9999998e10f;

	for (int i = 1; i <= gpGlobals->maxClients; ++i)
	{
		CBasePlayer *player = static_cast<CBasePlayer *>(UTIL_PlayerByIndex(i));

		if (!IsEntityValid(player))
			continue;

		if (player->IsAlive() && (team == UNASSIGNED || player->m_iTeam == team))
		{
			ShortestPathCost cost;
			Vector vecCenter = player->Center();

			float range = NavAreaTravelDistance(startArea, TheNavAreaGrid.GetNearestNavArea(&vecCenter), cost);

			if (range > 0 && range < closeRange)
			{
				closeRange = range;
				close = player;
			}
		}
	}
	return close;
}

struct CNpcGroup::impl_t
{
	std::vector<std::weak_ptr<CNpcGroupMember>> m_vecGroupMembers;
	Vector m_moveGoal;
	CNavPath m_path;
	CNavArea *m_startArea = nullptr;
	int m_iCurrentIndex = 0;

	float m_flNextTargetChange = gpGlobals->time + RANDOM_FLOAT(10.0f, 20.0f);
	float m_flNextThink = 0.0f;

	void Think()
	{
		if (gpGlobals->time < m_flNextThink)
			return;
		m_flNextThink = gpGlobals->time + 0.01f;

		if (gpGlobals->time > m_flNextTargetChange)
		{
			recomputePath();
			m_flNextTargetChange = gpGlobals->time + RANDOM_FLOAT(5.0f, 8.0f);
		}

		// TODO
	}

	bool recomputePath()
	{
		CBasePlayer *player = GetClosestPlayerByTravelDistance(m_startArea, CT);
		if (player)
		{
			HostagePathCost pathCost;
			Vector start = *m_startArea->GetCenter();
			m_moveGoal = player->Center();
			m_path.Compute(&start, &m_moveGoal, pathCost);
			m_iCurrentIndex = 0;
			return true;
		}
		return false;
	}

	// update startArea
	bool UpdateStartArea(const Vector &new_origin)
	{
		if (!m_path.GetSegmentCount())
		{
			m_flNextTargetChange = gpGlobals->time + 1.0f;
			return false;
		}

		assert(m_iCurrentIndex >= 0 && m_iCurrentIndex < m_path.GetSegmentCount());

		bool ret = false;
		for(int i = m_iCurrentIndex + 1; i < m_path.GetSegmentCount();++i)
		{
			if (m_path[i]->area->Contains(&new_origin))
			{
				m_startArea = m_path[i]->area;
				m_iCurrentIndex = i;
				ret = true;
				break;
			}
		}

		// repath ?
		if (m_iCurrentIndex == m_path.GetSegmentCount() - 1)
		{
			recomputePath();
		}

		return ret;
	}

	
};

CNpcGroup::CNpcGroup(const Vector &origin)
	: pimpl(std::make_unique<impl_t>()) 
{
	pimpl->m_startArea = TheNavAreaGrid.GetNearestNavArea(&origin);
	pimpl->recomputePath();
	assert(pimpl->m_startArea);
}

CNpcGroup::~CNpcGroup()
{
	// auto delete
	
}

std::shared_ptr<CNpcGroupMember> CNpcGroup::Join()
{
	auto ret = std::make_shared<CNpcGroupMember>(shared_from_this());
	pimpl->m_vecGroupMembers.emplace_back(ret);
	return ret;
}

bool CNpcGroup::IsCurrentAt(const Vector &vecWhere) const
{
	return pimpl->m_startArea != nullptr && pimpl->m_startArea->Contains(&vecWhere);
}

bool CNpcGroupMember::Apply(CHostageImprov *improv) 
{
	assert(improv != nullptr);

	if (improv->IsFollowing())
		return false;

	if (m_spGroup->pimpl->m_path.GetSegmentCount() <= 0)
		return false;

	// fast get near player

	improv->MoveTo(m_spGroup->pimpl->m_moveGoal);
	improv->SetFollowRange(9.9999998e10f, 3000.0f, 20.0f);

	// force repath
	CNavPath *pPath = improv->GetPath();
	if (pPath) *pPath = m_spGroup->pimpl->m_path;

	// report current status
	m_spGroup->pimpl->UpdateStartArea(improv->GetFeet());

	return true;
}

std::shared_ptr<CNpcGroupMember> GetGroup(const Vector &start)
{
	// ...
	static std::vector<std::weak_ptr<CNpcGroup>> TheNpcGroups;

	// optimize
	static int nOptCount = 0;
	if (++nOptCount > 31)
	{
		TheNpcGroups.erase(std::remove_if(TheNpcGroups.begin(), TheNpcGroups.end(), [](const std::weak_ptr<CNpcGroup> &wp) {
			return wp.expired();
		}), TheNpcGroups.end());
		nOptCount = 0;
	}

	// try to find a group
	std::shared_ptr<CNpcGroup> spDesiredGroup = nullptr;

	auto iter = std::find_if(TheNpcGroups.begin(), TheNpcGroups.end(), [&start](const std::weak_ptr<CNpcGroup> &wp) mutable {
		auto sp = wp.lock();
		// expired, remove it
		if (sp && sp->IsCurrentAt(start))
			return true;

		return false;
	});
	if (iter != TheNpcGroups.end())
		spDesiredGroup = iter->lock();

	// not found, create one! 
	if (!spDesiredGroup)
	{
		spDesiredGroup = std::make_shared<CNpcGroup>(start);
		TheNpcGroups.emplace_back(spDesiredGroup);
	}
	
	return spDesiredGroup->Join();
}