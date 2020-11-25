#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "animation.h"

namespace sv {

/*
* Globals initialization
*/
TYPEDESCRIPTION CBaseAnimating::m_SaveData[] =
		{
				DEFINE_FIELD(CBaseMonster, m_flFrameRate, FIELD_FLOAT),
				DEFINE_FIELD(CBaseMonster, m_flGroundSpeed, FIELD_FLOAT),
				DEFINE_FIELD(CBaseMonster, m_flLastEventCheck, FIELD_TIME),
				DEFINE_FIELD(CBaseMonster, m_fSequenceFinished, FIELD_BOOLEAN),
				DEFINE_FIELD(CBaseMonster, m_fSequenceLoops, FIELD_BOOLEAN),
		};

IMPLEMENT_SAVERESTORE(CBaseAnimating, CBaseDelay);

duration_t CBaseAnimating::StudioFrameAdvance(duration_t flInterval)
{
	if (flInterval == duration_t{}) {
		flInterval = gpGlobals->time - pev->animtime;

		if (flInterval <= 0.001s) {
			pev->animtime = gpGlobals->time;
			return 0s;
		}
	}

	if (pev->animtime == time_point_t{})
		flInterval = duration_t{};

	pev->frame += flInterval.count() * m_flFrameRate * pev->framerate;
	pev->animtime = gpGlobals->time;

	if (pev->frame < 0.0 || pev->frame >= 256.0) {
		if (m_fSequenceLoops)
			pev->frame -= (int) (pev->frame / 256.0) * 256.0;
		else
			pev->frame = (pev->frame < 0) ? 0 : 255;

		m_fSequenceFinished = TRUE;
	}

	return flInterval;
}

int CBaseAnimating::LookupActivity(int activity)
{
	void *pmodel = GET_MODEL_PTR(ENT(pev));
	return sv::LookupActivity(pmodel, pev, activity);
}

int CBaseAnimating::LookupActivityHeaviest(int activity)
{
	void *pmodel = GET_MODEL_PTR(ENT(pev));
	return sv::LookupActivityHeaviest(pmodel, pev, activity);
}

void CBaseAnimating::DispatchAnimEvents(duration_t flInterval)
{
	MonsterEvent_t event;
	void *pmodel = GET_MODEL_PTR(ENT(pev));

	if (!pmodel) {
		ALERT(at_aiconsole, "Gibbed monster is thinking!\n");
		return;
	}

	// FIXME: I have to do this or some events get missed, and this is probably causing the problem below
	flInterval = 0.1s;

	// FIX: this still sometimes hits events twice
	auto flStart = pev->frame + (m_flLastEventCheck - pev->animtime) * m_flFrameRate / 1s * pev->framerate;
	float flEnd = pev->frame + flInterval * m_flFrameRate / 1s * pev->framerate;

	m_fSequenceFinished = FALSE;
	m_flLastEventCheck = pev->animtime + flInterval;

	if (flEnd >= 256.0f || flEnd <= 0.0f) {
		m_fSequenceFinished = TRUE;
	}

	int index = 0;
	while ((index = GetAnimationEvent(pmodel, pev, &event, flStart, flEnd, index)) != 0) {
		HandleAnimEvent(&event);
	}
}

int CBaseAnimating::LookupSequence(const char *label)
{
	void *pmodel = GET_MODEL_PTR(ENT(pev));
	return sv::LookupSequence(pmodel, label);
}

void CBaseAnimating::ResetSequenceInfo()
{
	void *pmodel = GET_MODEL_PTR(ENT(pev));

	GetSequenceInfo(pmodel, pev, &m_flFrameRate, &m_flGroundSpeed);
	m_fSequenceLoops = ((GetSequenceFlags() & STUDIO_LOOPING) != 0);
	pev->animtime = gpGlobals->time;
	pev->framerate = 1.0f;

	m_fSequenceFinished = FALSE;
	m_flLastEventCheck = gpGlobals->time;
}

BOOL CBaseAnimating::GetSequenceFlags()
{
	void *pmodel = GET_MODEL_PTR(ENT(pev));
	return sv::GetSequenceFlags(pmodel, pev);
}

float CBaseAnimating::SetBoneController(int iController, float flValue)
{
	void *pmodel = GET_MODEL_PTR(ENT(pev));

	return SetController(pmodel, pev, iController, flValue);
}

void CBaseAnimating::InitBoneControllers()
{
	void *pmodel = GET_MODEL_PTR(ENT(pev));

	SetController(pmodel, pev, 0, 0);
	SetController(pmodel, pev, 1, 0);
	SetController(pmodel, pev, 2, 0);
	SetController(pmodel, pev, 3, 0);
}

NOXREF float CBaseAnimating::SetBlending(int iBlender, float flValue)
{
	void *pmodel = GET_MODEL_PTR(ENT(pev));
	return sv::SetBlending(pmodel, pev, iBlender, flValue);
}

NOXREF void CBaseAnimating::GetBonePosition(int iBone, Vector &origin, Vector &angles)
{
	GET_BONE_POSITION(ENT(pev), iBone, origin, angles);
}

NOXREF void CBaseAnimating::GetAttachment(int iAttachment, Vector &origin, Vector &angles)
{
	GET_ATTACHMENT(ENT(pev), iAttachment, origin, angles);
}

NOXREF int CBaseAnimating::FindTransition(int iEndingSequence, int iGoalSequence, int *piDir)
{
	void *pmodel = GET_MODEL_PTR(ENT(pev));

	if (piDir == NULL) {
		int iDir;
		int sequence = sv::FindTransition(pmodel, iEndingSequence, iGoalSequence, &iDir);

		if (iDir != 1)
			sequence = -1;

		return sequence;
	}

	return sv::FindTransition(pmodel, iEndingSequence, iGoalSequence, piDir);
}

NOXREF void CBaseAnimating::GetAutomovement(Vector &origin, Vector &angles, float flInterval)
{
	;
}

NOXREF void CBaseAnimating::SetBodygroup(int iGroup, int iValue)
{
	sv::SetBodygroup(GET_MODEL_PTR(ENT(pev)), pev, iGroup, iValue);
}

NOXREF int CBaseAnimating::GetBodygroup(int iGroup)
{
	return sv::GetBodygroup(GET_MODEL_PTR(ENT(pev)), pev, iGroup);
}

int CBaseAnimating::ExtractBbox(int sequence, float *mins, float *maxs)
{
	return sv::ExtractBbox(GET_MODEL_PTR(ENT(pev)), sequence, mins, maxs);
}

void CBaseAnimating::SetSequenceBox()
{
	Vector mins, maxs;

	// Get sequence bbox
	if (ExtractBbox(pev->sequence, mins, maxs)) {
		// expand box for rotation
		// find min / max for rotations
		float yaw = pev->angles.y * (M_PI / 180.0);

		Vector xvector, yvector;
		xvector.x = cos(yaw);
		xvector.y = sin(yaw);
		yvector.x = -sin(yaw);
		yvector.y = cos(yaw);

		Vector bounds[2];
		bounds[0] = mins;
		bounds[1] = maxs;

		Vector rmin(9999, 9999, 9999);
		Vector rmax(-9999, -9999, -9999);

		Vector base, transformed;
		for (int i = 0; i <= 1; ++i) {
			base.x = bounds[i].x;
			for (int j = 0; j <= 1; j++) {
				base.y = bounds[j].y;
				for (int k = 0; k <= 1; k++) {
					base.z = bounds[k].z;

					// transform the point
					transformed.x = xvector.x * base.x + yvector.x * base.y;
					transformed.y = xvector.y * base.x + yvector.y * base.y;
					transformed.z = base.z;

					if (transformed.x < rmin.x)
						rmin.x = transformed.x;

					if (transformed.x > rmax.x)
						rmax.x = transformed.x;

					if (transformed.y < rmin.y)
						rmin.y = transformed.y;

					if (transformed.y > rmax.y)
						rmax.y = transformed.y;

					if (transformed.z < rmin.z)
						rmin.z = transformed.z;

					if (transformed.z > rmax.z)
						rmax.z = transformed.z;
				}
			}
		}

		rmin.z = 0;
		rmax.z = rmin.z + 1;
		UTIL_SetSize(pev, rmin, rmax);
	}
}

}
