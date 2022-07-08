

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "animation.h"
#include "weapons.h"
#include "player.h"
#include "gamerules.h"
#include "training_gamerules.h"
#include "hltv.h"
#include "game.h"
#include "trains.h"
#include "vehicle.h"
#include "globals.h"

#include "bot_include.h"

#include "gamerules.h"
#include "career_tasks.h"
#include "maprules.h"

namespace sv {

void HostageAnimateState::Reset()
{
	m_sequenceCount = 0;
	m_performance = None;
}

void HostageAnimateState::StartSequence(CHostageImprov *improv, const SeqInfo *seqInfo)
{
	if (seqInfo->seqID >= 0)
	{
		CHostage *hostage = improv->GetEntity();

		hostage->pev->sequence = seqInfo->seqID;
		hostage->ResetSequenceInfo();
		hostage->m_fSequenceLoops = (seqInfo->holdTime > 0s) ? TRUE : FALSE;
		hostage->pev->frame = (seqInfo->rate < 0) ? 255 : 0;
		hostage->pev->framerate = seqInfo->rate * hostage->pev->framerate;
	}

	m_isHolding = false;
}

void HostageAnimateState::AddSequence(CHostageImprov *improv, const char *seqName, duration_t holdTime, float rate)
{
	int seqIndex;
	CHostage *hostage = improv->GetEntity();

	if (m_sequenceCount >= 8)
		return;

	if (seqName != NULL)
		seqIndex = hostage->LookupSequence(seqName);
	else
		seqIndex = -1;

	m_sequence[m_sequenceCount].seqID = seqIndex;
	m_sequence[m_sequenceCount].holdTime = holdTime;
	m_sequence[m_sequenceCount].rate = rate;

	m_currentSequence = 0;
	m_sequenceCount++;

	StartSequence(improv, m_sequence);
}

void HostageAnimateState::AddSequence(CHostageImprov *improv, int activity, duration_t holdTime, float rate)
{
	CHostage *hostage = improv->GetEntity();
	void *model = GET_MODEL_PTR(hostage->edict());

	if (model != NULL)
	{
		m_sequence[m_sequenceCount].seqID = LookupActivity(model, hostage->pev, activity);
		m_sequence[m_sequenceCount].holdTime = holdTime;
		m_sequence[m_sequenceCount].rate = rate;

		m_currentSequence = 0;
		m_sequenceCount++;
	}

	StartSequence(improv, m_sequence);
}

bool HostageAnimateState::IsDoneHolding()
{
	if (m_sequence[m_currentSequence].holdTime < 0s)
		return true;

	if (m_isHolding)
		return m_holdTimer.IsElapsed();

	m_isHolding = true;
	m_holdTimer.Start(m_sequence[m_currentSequence].holdTime);
	return false;
}

void HostageAnimateState::OnEnter(CHostageImprov *improv)
{
	;
}

void HostageAnimateState::OnUpdate(CHostageImprov *improv)
{
	if (m_sequenceCount <= 0)
		return;

	if (!improv->GetEntity()->m_fSequenceFinished && m_sequence[m_currentSequence].seqID >= 0)
		return;

	if (m_sequence[m_currentSequence].holdTime >= 0s)
	{
		if (!IsDoneHolding())
		{
			return;
		}
	}

	if (++m_currentSequence == m_sequenceCount)
	{
		m_sequenceCount = 0;
		m_performance = None;
		return;
	}

	StartSequence(improv, &m_sequence[m_currentSequence]);
}

void HostageAnimateState::OnExit(CHostageImprov *improv)
{
	;
}

NOXREF bool HostageAnimateState::IsPlaying(CHostageImprov *improv, const char *seqName) const
{
	int id = 0;
	CHostage *hostage = improv->GetEntity();

	if (m_sequenceCount > 0)
	{
		id = m_sequence[m_currentSequence].seqID;
	}

	return LookupSequence(hostage, seqName) == id;
}

}
