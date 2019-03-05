
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "trains.h"
#include "saverestore.h"
#include "pathcorner.h"

/*
* Globals initialization
*/
TYPEDESCRIPTION CPathCorner::m_SaveData[] =
{
	DEFINE_FIELD(CPathCorner, m_flWait, FIELD_FLOAT),
};

TYPEDESCRIPTION CPathTrack::m_SaveData[] =
{
	DEFINE_FIELD(CPathTrack, m_length, FIELD_FLOAT),
	DEFINE_FIELD(CPathTrack, m_pnext, FIELD_CLASSPTR),
	DEFINE_FIELD(CPathTrack, m_paltpath, FIELD_CLASSPTR),
	DEFINE_FIELD(CPathTrack, m_pprevious, FIELD_CLASSPTR),
	DEFINE_FIELD(CPathTrack, m_altName, FIELD_STRING),
};

LINK_ENTITY_TO_CLASS(path_corner, CPathCorner);

IMPLEMENT_SAVERESTORE(CPathCorner, CPointEntity);

void CPathCorner::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "wait"))
	{
		m_flWait = Q_atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CPointEntity::KeyValue(pkvd);
}

void CPathCorner::Spawn()
{
	assert(("path_corner without a targetname" && !FStringNull(pev->targetname)));
}

IMPLEMENT_SAVERESTORE(CPathTrack, CBaseEntity);

LINK_ENTITY_TO_CLASS(path_track, CPathTrack);

void CPathTrack::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "altpath"))
	{
		m_altName = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CPointEntity::KeyValue(pkvd);
}

void CPathTrack::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	int on;

	// Use toggles between two paths
	if (m_paltpath)
	{
		on = !(pev->spawnflags & SF_PATH_ALTERNATE);

		if (ShouldToggle(useType, on))
		{
			if (on)
				pev->spawnflags |= SF_PATH_ALTERNATE;
			else
				pev->spawnflags &= ~SF_PATH_ALTERNATE;
		}
	}
	else	// Use toggles between enabled/disabled
	{
		on = !(pev->spawnflags & SF_PATH_DISABLED);

		if (ShouldToggle(useType, on))
		{
			if (on)
				pev->spawnflags |= SF_PATH_DISABLED;
			else
				pev->spawnflags &= ~SF_PATH_DISABLED;
		}
	}
}

void CPathTrack::Link()
{
	edict_t *pentTarget;

	if (!FStringNull(pev->target))
	{
		pentTarget = FIND_ENTITY_BY_TARGETNAME(NULL, STRING(pev->target));
		if (!FNullEnt(pentTarget))
		{
			m_pnext = CPathTrack::Instance(pentTarget);

			// If no next pointer, this is the end of a path
			if (m_pnext != NULL)
			{
				m_pnext->SetPrevious(this);
			}
		}
		else
			ALERT(at_console, "Dead end link %s\n", STRING(pev->target));
	}

	// Find "alternate" path
	if (!FStringNull(m_altName))
	{
		pentTarget = FIND_ENTITY_BY_TARGETNAME(NULL, STRING(m_altName));
		if (!FNullEnt(pentTarget))
		{
			m_paltpath = CPathTrack::Instance(pentTarget);

			// If no next pointer, this is the end of a path
			if (m_paltpath != NULL)
			{
				m_paltpath->SetPrevious(this);
			}
		}
	}
}

void CPathTrack::Spawn()
{
	pev->solid = SOLID_TRIGGER;
	UTIL_SetSize(pev, Vector(-8, -8, -8), Vector(8, 8, 8));

	m_pnext = NULL;
	m_pprevious = NULL;
}

void CPathTrack::Activate()
{
	// Link to next, and back-link
	if (!FStringNull(pev->targetname))
	{
		Link();
	}
}

CPathTrack *CPathTrack::ValidPath(CPathTrack *ppath, int testFlag)
{
	if (!ppath)
		return NULL;

	if (testFlag && (ppath->pev->spawnflags & SF_PATH_DISABLED))
		return NULL;

	return ppath;
}

void CPathTrack::Project(CPathTrack *pstart, CPathTrack *pend, Vector *origin, float dist)
{
	if (pstart && pend)
	{
		Vector dir = (pend->pev->origin - pstart->pev->origin);
		dir = dir.Normalize();

		*origin = pend->pev->origin + dir * dist;
	}
}

CPathTrack *CPathTrack::GetNext()
{
	if (m_paltpath && (pev->spawnflags & SF_PATH_ALTERNATE) && !(pev->spawnflags & SF_PATH_ALTREVERSE))
	{
		return m_paltpath;
	}

	return m_pnext;
}

CPathTrack *CPathTrack::GetPrevious()
{
	if (m_paltpath && (pev->spawnflags & SF_PATH_ALTERNATE) && (pev->spawnflags & SF_PATH_ALTREVERSE))
	{
		return m_paltpath;
	}

	return m_pprevious;
}

void CPathTrack::SetPrevious(CPathTrack *pprev)
{
	// Only set previous if this isn't my alternate path
	if (pprev && !FStrEq(STRING(pprev->pev->targetname), STRING(m_altName)))
	{
		m_pprevious = pprev;
	}
}

// Assumes this is ALWAYS enabled

CPathTrack *CPathTrack::LookAhead(Vector *origin, float dist, int move)
{
	CPathTrack *pcurrent;
	float originalDist = dist;

	pcurrent = this;
	Vector currentPos = *origin;

	// Travelling backwards through path
	if (dist < 0)
	{
		dist = -dist;
		while (dist > 0)
		{
			Vector dir = pcurrent->pev->origin - currentPos;
			float length = dir.Length();

			if (!length)
			{
				// If there is no previous node, or it's disabled, return now.
				if (!ValidPath(pcurrent->GetPrevious(), move))
				{
					if (!move)
					{
						Project(pcurrent->GetNext(), pcurrent, origin, dist);
					}

					return NULL;
				}

				pcurrent = pcurrent->GetPrevious();
			}
			// enough left in this path to move
			else if (length > dist)
			{
				*origin = currentPos + (dir * ((float)(dist / length)));
				return pcurrent;
			}
			else
			{
				dist -= length;
				currentPos = pcurrent->pev->origin;
				*origin = currentPos;

				// If there is no previous node, or it's disabled, return now.
				if (!ValidPath(pcurrent->GetPrevious(), move))
				{
					return NULL;
				}

				pcurrent = pcurrent->GetPrevious();
			}
		}

		*origin = currentPos;
		return pcurrent;
	}
	else
	{
		// #96 line
		while (dist > 0)
		{
			// If there is no next node, or it's disabled, return now.
			if (!ValidPath(pcurrent->GetNext(), move))
			{
				if (!move)
				{
					Project(pcurrent->GetPrevious(), pcurrent, origin, dist);
				}

				return NULL;
			}

			Vector dir = pcurrent->GetNext()->pev->origin - currentPos;
			float length = dir.Length();

			if (!length  && !ValidPath(pcurrent->GetNext()->GetNext(), move))
			{
				// HACK -- up against a dead end
				if (dist == originalDist)
					return NULL;

				return pcurrent;
			}

			// enough left in this path to move
			if (length > dist)
			{
				*origin = currentPos + (dir * ((float)(dist / length)));
				return pcurrent;
			}
			else
			{
				dist -= length;
				currentPos = pcurrent->GetNext()->pev->origin;
				pcurrent = pcurrent->GetNext();

				*origin = currentPos;
			}
		}

		*origin = currentPos;
	}

	return pcurrent;
}

// Assumes this is ALWAYS enabled

CPathTrack *CPathTrack::Nearest(Vector origin)
{
	int deadCount;
	float minDist, dist;
	Vector delta;
	CPathTrack *ppath, *pnearest;

	delta = origin - pev->origin;
	delta.z = 0;
	minDist = delta.Length();
	pnearest = this;
	ppath = GetNext();

	// Hey, I could use the old 2 racing pointers solution to this, but I'm lazy :)
	deadCount = 0;
	while (ppath != NULL && ppath != this)
	{
		if (++deadCount > 9999)
		{
			ALERT(at_error, "Bad sequence of path_tracks from %s", STRING(pev->targetname));
			return NULL;
		}

		delta = origin - ppath->pev->origin;
		delta.z = 0;
		dist = delta.Length();

		if (dist < minDist)
		{
			minDist = dist;
			pnearest = ppath;
		}

		ppath = ppath->GetNext();
	}

	return pnearest;
}

CPathTrack *CPathTrack::Instance(edict_t *pent)
{
	if (FClassnameIs(pent, "path_track"))
	{
		return (CPathTrack *)GET_PRIVATE(pent);
	}

	return NULL;
}
