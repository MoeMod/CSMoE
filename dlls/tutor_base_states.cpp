
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "saverestore.h"
#include "globals.h"
#include "game.h"

#include "bot_include.h"

namespace sv {

CBaseTutorStateSystem::CBaseTutorStateSystem()
{
	;
}

CBaseTutorStateSystem::~CBaseTutorStateSystem()
{
	;
}

int CBaseTutorStateSystem::GetCurrentStateType() const
{
	if (m_currentState != NULL)
	{
		return m_currentState->GetType();
	}

	return 0;
}

CBaseTutorState::CBaseTutorState()
{
	;
}

CBaseTutorState::~CBaseTutorState()
{
	;
}

int CBaseTutorState::GetType() const
{
	return m_type;
}

}
