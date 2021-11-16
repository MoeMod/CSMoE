#include "bot_include.h"

namespace sv {

// Begin defusing the bomb

void DefendState::OnEnter(CCSBot* me)
{
	me->DestroyPath();
	m_checkInterval.Start(2.0s);
}

// Defuse the bomb

void DefendState::OnUpdate(CCSBot* me)
{
	// look around
	me->UpdateLookAround();

	if (!m_checkInterval.IsElapsed())
		return;

	if (!m_defendArea)
	{
		me->Idle();
		return;
	}

	m_checkInterval.Start(2.0s);
	if (m_defendArea->Contains(&me->pev->origin))
	{
		return;
	}

	Vector pos;
	m_defendArea->GetClosestPointOnArea(&me->pev->origin, &pos);
	if ((pos - me->pev->origin).IsLengthGreaterThan(700.0f))
	{
		me->Idle();
		return;
	}

	me->SetTask(CCSBot::MOVE_TO_SAFE_AREA);
	me->MoveTo(m_defendArea->GetCenter());
}

void DefendState::OnExit(CCSBot *me)
{
	m_defendArea = nullptr;
}

}
