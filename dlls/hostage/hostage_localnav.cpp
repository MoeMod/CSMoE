

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "player.h"
#include "gamerules.h"
#include "training_gamerules.h"
#include "hltv.h"
#include "game.h"
#include "trains.h"
#include "vehicle.h"
#include "globals.h"

#include "pm_shared.h"
#include "utllinkedlist.h"

// CSBOT and Nav
#include "game_shared/GameEvent.h"		// Game event enum used by career mode, tutor system, and bots
#include "game_shared/bot/bot_util.h"
#include "game_shared/bot/simple_state_machine.h"

#include "game_shared/steam_util.h"

#include "game_shared/bot/bot_manager.h"
#include "game_shared/bot/bot_constants.h"
#include "game_shared/bot/bot.h"

#include "game_shared/shared_util.h"
#include "game_shared/bot/bot_profile.h"

#include "game_shared/bot/nav.h"
#include "game_shared/bot/improv.h"
#include "game_shared/bot/nav_node.h"
#include "game_shared/bot/nav_area.h"
#include "game_shared/bot/nav_file.h"
#include "game_shared/bot/nav_path.h"

#include "airtank.h"
#include "h_ai.h"
#include "h_cycler.h"
#include "h_battery.h"

// Hostage
#include "hostage/hostage.h"
#include "hostage/hostage_localnav.h"

#include "bot/cs_bot.h"

// Tutor
#include "tutor.h"
#include "tutor_base_states.h"
#include "tutor_base_tutor.h"
#include "tutor_cs_states.h"
#include "tutor_cs_tutor.h"

#include "gamerules.h"
#include "career_tasks.h"
#include "maprules.h"

/*
* Globals initialization
*/
float CLocalNav::s_flStepSize;
int CLocalNav::qptr;
EHANDLE CLocalNav::_queue[MAX_HOSTAGES_NAV];
int CLocalNav::tot_inqueue;
float CLocalNav::nodeval;
float CLocalNav::flNextCvarCheck;
float CLocalNav::flLastThinkTime;
EHANDLE CLocalNav::hostages[MAX_HOSTAGES_NAV];
int CLocalNav::tot_hostages;

CLocalNav::CLocalNav(CHostage *pOwner)
{
	m_pOwner = pOwner;
	m_pTargetEnt = NULL;
	m_nodeArr = new localnode_t[MAX_NODES];

	if (tot_hostages >= MAX_HOSTAGES_NAV)
	{
		return;
	}

	hostages[tot_hostages++] = pOwner;
}

CLocalNav::~CLocalNav()
{
	delete m_nodeArr;
}

node_index_t CLocalNav::AddNode(node_index_t nindexParent, Vector &vecLoc, int offsetX, int offsetY, byte bDepth)
{
	localnode_t *nodeNew;

	if (m_nindexAvailableNode == MAX_NODES)
		return -1;

	nodeNew = GetNode(m_nindexAvailableNode);

	nodeNew->vecLoc = vecLoc;
	nodeNew->offsetX = offsetX;
	nodeNew->offsetY = offsetY;
	nodeNew->bDepth = bDepth;
	nodeNew->fSearched = FALSE;
	nodeNew->nindexParent = nindexParent;

	return m_nindexAvailableNode++;
}

localnode_t *CLocalNav::GetNode(node_index_t nindex)
{
	return &m_nodeArr[ nindex ];
}

node_index_t CLocalNav::NodeExists(int offsetX, int offsetY)
{
	node_index_t nindexCurrent = NODE_INVALID_EMPTY;
	localnode_t *nodeCurrent;

	for (nindexCurrent = m_nindexAvailableNode - 1; nindexCurrent != NODE_INVALID_EMPTY; nindexCurrent--)
	{
		nodeCurrent = GetNode(nindexCurrent);

		if (nodeCurrent->offsetX == offsetX && nodeCurrent->offsetY == offsetY)
		{
			break;
		}
	}

	return nindexCurrent;
}

void CLocalNav::AddPathNodes(node_index_t nindexSource, int fNoMonsters)
{
	AddPathNode(nindexSource, 1, 0, fNoMonsters);
	AddPathNode(nindexSource, -1, 0, fNoMonsters);
	AddPathNode(nindexSource, 0, 1, fNoMonsters);
	AddPathNode(nindexSource, 0, -1, fNoMonsters);
	AddPathNode(nindexSource, 1, 1, fNoMonsters);
	AddPathNode(nindexSource, 1, -1, fNoMonsters);
	AddPathNode(nindexSource, -1, 1, fNoMonsters);
	AddPathNode(nindexSource, -1, -1, fNoMonsters);
}

void CLocalNav::AddPathNode(node_index_t nindexSource, int offsetX, int offsetY, int fNoMonsters)
{
	int bDepth;
	Vector vecSource, vecDest;
	int offsetXAbs, offsetYAbs;

	if (nindexSource == -1)
	{
		bDepth = 1;

		offsetXAbs = offsetX;
		offsetYAbs = offsetY;

		vecSource = m_vecStartingLoc;
		vecDest = vecSource + Vector(((float)offsetX * HOSTAGE_STEPSIZE), ((float)offsetY * HOSTAGE_STEPSIZE), 0);
	}
	else
	{
		localnode_t *nodeSource;
		localnode_t *nodeCurrent;
		node_index_t nindexCurrent;

		nodeCurrent = GetNode(nindexSource);
		offsetXAbs = offsetX + nodeCurrent->offsetX;
		offsetYAbs = offsetY + nodeCurrent->offsetY;
		nodeSource = GetNode(m_nindexAvailableNode);

		// if there exists a node, then to ignore adding a the new node
		if (NodeExists(offsetXAbs, offsetYAbs) != NODE_INVALID_EMPTY)
		{
			return;
		}

		vecSource = nodeCurrent->vecLoc;
		vecDest = vecSource + Vector(((float)offsetX * HOSTAGE_STEPSIZE), ((float)offsetY * HOSTAGE_STEPSIZE), 0);

		if (m_nindexAvailableNode)
		{
			nindexCurrent = m_nindexAvailableNode;

			do
			{
				nodeSource--;
				nindexCurrent--;

				offsetX = (nodeSource->offsetX - offsetXAbs);

				if (offsetX >= 0)
				{
					if (offsetX > 1)
					{
						continue;
					}
				}
				else
				{
					if (-offsetX > 1)
					{
						continue;
					}
				}

				offsetY = (nodeSource->offsetY - offsetYAbs);

				if (offsetY >= 0)
				{
					if (offsetY > 1)
					{
						continue;
					}
				}
				else
				{
					if (-offsetY > 1)
					{
						continue;
					}
				}

				if (PathTraversable(nodeSource->vecLoc, vecDest, fNoMonsters) != PATH_TRAVERSABLE_EMPTY)
				{
					nodeCurrent = nodeSource;
					nindexSource = nindexCurrent;
				}
			}
			while (nindexCurrent);
		}

		vecSource = nodeCurrent->vecLoc;
		bDepth = ((int)nodeCurrent->bDepth) + 1;
	}

	if (PathTraversable(vecSource, vecDest, fNoMonsters) != PATH_TRAVERSABLE_EMPTY)
	{
		AddNode(nindexSource, vecDest, offsetXAbs, offsetYAbs, bDepth);
	}
}

node_index_t CLocalNav::GetBestNode(Vector &vecOrigin, Vector &vecDest)
{
	node_index_t nindexCurrent;
	localnode_t *nodeCurrent;
	node_index_t nindexBest;
	float flBestVal;

	nindexBest = -1;
	nindexCurrent = 0;
	flBestVal = 1000000.0;

	while (nindexCurrent < m_nindexAvailableNode)
	{
		nodeCurrent = GetNode(nindexCurrent);

		if (!nodeCurrent->fSearched)
		{
			float flCurrentVal;
			float flDistFromStart;

			float flDistToDest;
			float flZDiff = -1.0;

			flDistFromStart = LengthSubtract
				<float, float,
				float, float>(vecDest, nodeCurrent->vecLoc);

			flDistToDest = nodeCurrent->vecLoc.z - vecDest.z;
			if (flDistToDest >= 0.0)
			{
				flZDiff = 1.0;
			}

			if ((flDistToDest * flZDiff) <= s_flStepSize)
				flZDiff = 1.0;
			else
				flZDiff = 1.25;

			flCurrentVal = flZDiff * (((float)nodeCurrent->bDepth * HOSTAGE_STEPSIZE) + flDistFromStart);
			if (flCurrentVal < flBestVal)
			{
				flBestVal = flCurrentVal;
				nindexBest = nindexCurrent;
			}
		}

		++nindexCurrent;
	}

	return nindexBest;
}

int CLocalNav::SetupPathNodes(node_index_t nindex, Vector *vecNodes, int fNoMonsters)
{
	node_index_t nCurrentIndex = nindex;
	int nNodeCount = 0;

	while (nCurrentIndex != -1)
	{
		localnode_t *nodeCurrent = GetNode(nCurrentIndex);
		Vector vecCurrentLoc = nodeCurrent->vecLoc;
		vecNodes[ nNodeCount++ ] = vecCurrentLoc;

		nCurrentIndex = nodeCurrent->nindexParent;
	}

	return nNodeCount;
}

int CLocalNav::GetFurthestTraversableNode(Vector &vecStartingLoc, Vector *vecNodes, int nTotalNodes, int fNoMonsters)
{
	int nCount = 0;
	while (nCount < nTotalNodes)
	{
		if (PathTraversable(vecStartingLoc, vecNodes[nCount], fNoMonsters) != PATH_TRAVERSABLE_EMPTY)
			return nCount;

		++nCount;
	}

	return -1;
}

node_index_t CLocalNav::FindPath(Vector &vecStart, Vector &vecDest, float flTargetRadius, int fNoMonsters)
{
	node_index_t nIndexBest = FindDirectPath(vecStart, vecDest, flTargetRadius, fNoMonsters);

	if (nIndexBest != -1)
	{
		return nIndexBest;
	}

	localnode_t *node;
	Vector vecNodeLoc;
	float flDistToDest;

	m_vecStartingLoc = vecStart;
	m_nindexAvailableNode = 0;

	AddPathNodes(-1, fNoMonsters);
	nIndexBest = GetBestNode(vecStart, vecDest);

	while (nIndexBest != -1)
	{
		node = GetNode(nIndexBest);
		node->fSearched = TRUE;

		vecNodeLoc = node->vecLoc;
		flDistToDest = (vecDest - node->vecLoc).Length2D();

		if (flDistToDest <= flTargetRadius)
			break;

		if (flDistToDest <= HOSTAGE_STEPSIZE)
			break;

		if (((flDistToDest - flTargetRadius) > ((MAX_NODES - m_nindexAvailableNode) * HOSTAGE_STEPSIZE))
			|| m_nindexAvailableNode == MAX_NODES)
		{
			nIndexBest = -1;
			break;
		}

		AddPathNodes(nIndexBest, fNoMonsters);
		nIndexBest = GetBestNode(vecNodeLoc, vecDest);
	}

	if (m_nindexAvailableNode <= 10)
		nodeval += 2;

	else if (m_nindexAvailableNode <= 20)
		nodeval += 4;

	else if (m_nindexAvailableNode <= 30)
		nodeval += 8;

	else if (m_nindexAvailableNode <= 40)
		nodeval += 13;

	else if (m_nindexAvailableNode <= 50)
		nodeval += 19;

	else if (m_nindexAvailableNode <= 60)
		nodeval += 26;

	else if (m_nindexAvailableNode <= 70)
		nodeval += 34;

	else if (m_nindexAvailableNode <= 80)
		nodeval += 43;

	else if (m_nindexAvailableNode <= 90)
		nodeval += 53;

	else if (m_nindexAvailableNode <= 100)
		nodeval += 64;

	else if (m_nindexAvailableNode <= 110)
		nodeval += 76;

	else if (m_nindexAvailableNode <= 120)
		nodeval += 89;

	else if (m_nindexAvailableNode <= 130)
		nodeval += 103;

	else if (m_nindexAvailableNode <= 140)
		nodeval += 118;

	else if (m_nindexAvailableNode <= 150)
		nodeval += 134;

	else if (m_nindexAvailableNode <= 160)
		nodeval += 151;
	else
		nodeval += 169;

	return nIndexBest;
}

node_index_t CLocalNav::FindDirectPath(Vector &vecStart, Vector &vecDest, float flTargetRadius, int fNoMonsters)
{
	Vector vecActualDest;
	Vector vecPathDir;
	Vector vecNodeLoc;
	node_index_t nindexLast;

	vecPathDir = NormalizeSubtract<float, float, float, float>(vecStart, vecDest);
	vecActualDest = vecDest - (vecPathDir * flTargetRadius);

	if (PathTraversable(vecStart, vecActualDest, fNoMonsters) == PATH_TRAVERSABLE_EMPTY)
	{
		return -1;
	}

	nindexLast = -1;
	vecNodeLoc = vecStart;
	m_nindexAvailableNode = 0;

	while ((vecNodeLoc - vecActualDest).Length2D() >= HOSTAGE_STEPSIZE)
	{
		node_index_t nindexCurrent = nindexLast;

		vecNodeLoc = vecNodeLoc + (vecPathDir * HOSTAGE_STEPSIZE);
		nindexLast = AddNode(nindexCurrent, vecNodeLoc);

		if (nindexLast == -1)
			break;
	}

	return nindexLast;
}

BOOL CLocalNav::PathClear(Vector &vecOrigin, Vector &vecDest, int fNoMonsters, TraceResult &tr)
{
	TRACE_MONSTER_HULL(m_pOwner->edict(), vecOrigin, vecDest, fNoMonsters, m_pOwner->edict(), &tr);

	if (tr.fStartSolid)
		return FALSE;

	if (tr.flFraction == 1.0f)
		return TRUE;

	if (tr.pHit == m_pTargetEnt)
	{
		m_fTargetEntHit = TRUE;
		return TRUE;
	}

	return FALSE;
}

int CLocalNav::PathTraversable(Vector &vecSource, Vector &vecDest, int fNoMonsters)
{
	TraceResult tr;
	Vector vecSrcTmp;
	Vector vecDestTmp;
	Vector vecDir;
	float flTotal;
	int retval = PATH_TRAVERSABLE_EMPTY;

	vecSrcTmp = vecSource;
	vecDestTmp = vecDest - vecSource;

	vecDir = vecDestTmp.NormalizePrecision();
	vecDir.z = 0;

	flTotal = vecDestTmp.Length2D();

	while (flTotal > 1.0)
	{
		if (flTotal >= s_flStepSize)
		{
			vecDestTmp = vecSrcTmp + (vecDir * s_flStepSize);
		}
		else
			vecDestTmp = vecDest;

		m_fTargetEntHit = FALSE;

		if (PathClear(vecSrcTmp, vecDestTmp, fNoMonsters, tr))
		{
			vecDestTmp = tr.vecEndPos;

			if (retval == PATH_TRAVERSABLE_EMPTY)
			{
				retval = PATH_TRAVERSABLE_SLOPE;
			}
		}
		else
		{
			if (tr.fStartSolid)
			{
				return PATH_TRAVERSABLE_EMPTY;
			}

			if (tr.pHit && !fNoMonsters && tr.pHit->v.classname)
			{
				if (FClassnameIs(tr.pHit, "hostage_entity"))
				{
					return PATH_TRAVERSABLE_EMPTY;
				}
			}

			vecSrcTmp = tr.vecEndPos;

			if (tr.vecPlaneNormal.z <= 0.7)
			{
				if (StepTraversable(vecSrcTmp, vecDestTmp, fNoMonsters, tr))
				{
					if (retval == PATH_TRAVERSABLE_EMPTY)
					{
						retval = PATH_TRAVERSABLE_STEP;
					}
				}
				else
				{
					if (!StepJumpable(vecSrcTmp, vecDestTmp, fNoMonsters, tr))
					{
						return PATH_TRAVERSABLE_EMPTY;
					}

					if (retval == PATH_TRAVERSABLE_EMPTY)
					{
						retval = PATH_TRAVERSABLE_STEPJUMPABLE;
					}
				}
			}
			else
			{
				if (!SlopeTraversable(vecSrcTmp, vecDestTmp, fNoMonsters, tr))
				{
					return PATH_TRAVERSABLE_EMPTY;
				}

				if (retval == PATH_TRAVERSABLE_EMPTY)
				{
					retval = PATH_TRAVERSABLE_SLOPE;
				}
			}
		}

		Vector vecDropDest = vecDestTmp - Vector(0, 0, 300);

		if (PathClear(vecDestTmp, vecDropDest, fNoMonsters, tr))
		{
			return PATH_TRAVERSABLE_EMPTY;
		}

		if (!tr.fStartSolid)
			vecDestTmp = tr.vecEndPos;

		vecSrcTmp = vecDestTmp;

		BOOL fProgressThisTime = m_fTargetEntHit;
		Vector vecSrcThisTime = vecDest - vecDestTmp;

		if (fProgressThisTime)
			break;

		flTotal = vecSrcThisTime.Length2D();
	}

	vecDest = vecDestTmp;

	return retval;
}

BOOL CLocalNav::SlopeTraversable(Vector &vecSource, Vector &vecDest, int fNoMonsters, TraceResult &tr)
{
	Vector vecSlopeEnd;
	Vector vecDown;
	Vector vecAngles;

	vecSlopeEnd = vecDest;
	vecDown = vecDest - vecSource;

	vecAngles = UTIL_VecToAngles(tr.vecPlaneNormal);
	vecSlopeEnd.z = vecDown.Length2D() * tan((float)((90.0 - vecAngles.x) * (M_PI / 180))) + vecSource.z;

	if (!PathClear(vecSource, vecSlopeEnd, fNoMonsters, tr))
	{
		if (tr.fStartSolid)
			return FALSE;

		if ((tr.vecEndPos - vecSource).Length2D() < 1.0)
			return FALSE;
	}

	vecSlopeEnd = tr.vecEndPos;

	vecDown = vecSlopeEnd;
	vecDown.z -= s_flStepSize;

	if (!PathClear(vecSlopeEnd, vecDown, fNoMonsters, tr))
	{
		if (tr.fStartSolid)
		{
			vecDest = vecSlopeEnd;
			return TRUE;
		}
	}

	vecDest = tr.vecEndPos;
	return TRUE;
}

BOOL CLocalNav::LadderTraversable(Vector &vecSource, Vector &vecDest, int fNoMonsters, TraceResult &tr)
{
	Vector vecStepStart;
	Vector vecStepDest;

	vecStepStart = tr.vecEndPos;
	vecStepDest = vecStepStart;
	vecStepDest.z += HOSTAGE_STEPSIZE;

	if (!PathClear(vecStepStart, vecStepDest, fNoMonsters, tr))
	{
		if (tr.fStartSolid)
			return FALSE;

		if ((tr.vecEndPos - vecStepStart).Length() < 1)
			return FALSE;
	}

	vecStepStart = tr.vecEndPos;
	vecDest.z = tr.vecEndPos.z;

	return PathTraversable(vecStepStart, vecDest, fNoMonsters);
}

BOOL CLocalNav::StepTraversable(Vector &vecSource, Vector &vecDest, int fNoMonsters, TraceResult &tr)
{
	Vector vecStepStart;
	Vector vecStepDest;
	//BOOL fFwdTrace = FALSE; // unused?
	float flFwdFraction;

	vecStepStart = vecSource;
	vecStepDest = vecDest;

	vecStepStart.z += s_flStepSize;
	vecStepDest.z = vecStepStart.z;

	if (!PathClear(vecStepStart, vecStepDest, fNoMonsters, tr))
	{
		if (tr.fStartSolid)
			return FALSE;

		flFwdFraction = (tr.vecEndPos - vecStepStart).Length();

		if (flFwdFraction < 1.0)
			return FALSE;
	}

	vecStepStart = tr.vecEndPos;

	vecStepDest = vecStepStart;
	vecStepDest.z -= s_flStepSize;

	if (!PathClear(vecStepStart, vecStepDest, fNoMonsters, tr))
	{
		if (tr.fStartSolid)
		{
			vecDest = vecStepStart;
			return TRUE;
		}
	}

	vecDest = tr.vecEndPos;
	return TRUE;
}

BOOL CLocalNav::StepJumpable(Vector &vecSource, Vector &vecDest, int fNoMonsters, TraceResult &tr)
{
	Vector vecStepStart;
	Vector vecStepDest;
	//BOOL fFwdTrace = FALSE; // unused?
	float flFwdFraction;
	float flJumpHeight = s_flStepSize + 1.0;
	//BOOL fJumpClear = FALSE; // unused?
	//edict_t *hit = NULL; // unused?

	vecStepStart = vecSource;
	vecStepStart.z += flJumpHeight;

	while (flJumpHeight < 40.0)
	{
		vecStepDest = vecDest;
		vecStepDest.z = vecStepStart.z;

		if (!PathClear(vecStepStart, vecStepDest, fNoMonsters, tr))
		{
			if (tr.fStartSolid)
				break;

			flFwdFraction = (tr.vecEndPos - vecStepStart).Length2D();

			if (flFwdFraction < 1.0)
			{
				flJumpHeight += 10.0;
				vecStepStart.z += 10.0;

				continue;
			}
		}

		vecStepStart = tr.vecEndPos;
		vecStepDest = vecStepStart;
		vecStepDest.z -= s_flStepSize;

		if (!PathClear(vecStepStart, vecStepDest, fNoMonsters, tr))
		{
			if (tr.fStartSolid)
			{
				vecDest = vecStepStart;
				return TRUE;
			}
		}

		vecDest = tr.vecEndPos;
		return TRUE;
	}

	return FALSE;
}

BOOL CLocalNav::LadderHit(Vector &vecSource, Vector &vecDest, TraceResult &tr)
{
	Vector vecFwd, vecRight, vecUp;
	Vector vecAngles, vecOrigin;

	vecAngles = UTIL_VecToAngles(-tr.vecPlaneNormal);
	UTIL_MakeVectorsPrivate(vecAngles, vecFwd, vecRight, vecUp);
	vecOrigin = tr.vecEndPos + (vecFwd * 15) + (vecUp * 36);

	if (UTIL_PointContents(vecOrigin) == CONTENTS_LADDER)
		return true;

	vecOrigin = tr.vecEndPos + (vecFwd * 15) - (vecUp * 36);

	if (UTIL_PointContents(vecOrigin) == CONTENTS_LADDER)
		return true;

	vecOrigin = tr.vecEndPos + (vecFwd * 15) + (vecRight * 16) + (vecUp * 36);

	if (UTIL_PointContents(vecOrigin) == CONTENTS_LADDER)
		return true;

	vecOrigin = tr.vecEndPos + (vecFwd * 15) - (vecRight * 16) + (vecUp * 36);

	if (UTIL_PointContents(vecOrigin) == CONTENTS_LADDER)
		return true;

	vecOrigin = tr.vecEndPos + (vecFwd * 15) + (vecRight * 16) - (vecUp * 36);

	if (UTIL_PointContents(vecOrigin) == CONTENTS_LADDER)
		return true;

	vecOrigin = tr.vecEndPos + (vecFwd * 15) - (vecRight * 16) + (vecUp * 36);

	if (UTIL_PointContents(vecOrigin) == CONTENTS_LADDER)
		return true;

	return false;
}

void CLocalNav::Think()
{
	EHANDLE hCallback;
	static cvar_t *sv_stepsize = NULL;

	if (gpGlobals->time >= flNextCvarCheck)
	{
		if (sv_stepsize != NULL)
			s_flStepSize = sv_stepsize->value;
		else
		{
			sv_stepsize = CVAR_GET_POINTER("sv_stepsize");
			s_flStepSize = s_flStepSize ? sv_stepsize->value : HOSTAGE_STEPSIZE_DEFAULT;
		}

		flNextCvarCheck = gpGlobals->time + 1;
	}

	HostagePrethink();

	float flElapsedTime = gpGlobals->time - flLastThinkTime;
	nodeval -= flElapsedTime * 250;
	flLastThinkTime = gpGlobals->time;

	if (nodeval < 0)
		nodeval = 0;

	else if (nodeval > 17)
		return;

	if (tot_inqueue)
	{
		hCallback = _queue[qptr];

		if (!hCallback)
		{
			while (tot_inqueue > 0)
			{
				if (++qptr == MAX_HOSTAGES_NAV)
					qptr = 0;

				tot_inqueue--;
				if (!tot_inqueue)
				{
					hCallback = NULL;
					break;
				}

				hCallback = _queue[qptr];

				if (hCallback)
					break;
			}
		}

		if (hCallback)
		{
			CHostage *pHostage = GetClassPtr<CHostage>(hCallback->pev);

			if (++qptr == MAX_HOSTAGES_NAV)
				qptr = 0;

			tot_inqueue--;
			pHostage->NavReady();
		}
	}
}

void CLocalNav::RequestNav(CHostage *pCaller)
{
	int curr = qptr;
	int found = 0;

	if (nodeval <= 17 && !tot_inqueue)
	{
		pCaller->NavReady();
		return;
	}

	if (tot_inqueue >= MAX_HOSTAGES_NAV)
	{
		return;
	}

	for (int i = 0; i < tot_inqueue; ++i)
	{
		CHostage *pQueueItem = GetClassPtr<CHostage>(_queue[curr]->pev);

		if (pQueueItem == pCaller)
			return;

		if (++curr == MAX_HOSTAGES_NAV)
			curr = 0;
	}

	_queue[curr] = pCaller;
	++tot_inqueue;
}

void CLocalNav::Reset()
{
	flNextCvarCheck = 0;
	flLastThinkTime = 0;
	tot_inqueue = 0;
	qptr = 0;
	nodeval = 0;
	tot_hostages = 0;
}

void CLocalNav::HostagePrethink()
{
	for (int iCount = 0; iCount < tot_hostages; ++iCount)
	{
		if (hostages[ iCount ] != NULL)
		{
			GetClassPtr<CHostage>(hostages[ iCount ]->pev)->PreThink();
		}
	}
}
