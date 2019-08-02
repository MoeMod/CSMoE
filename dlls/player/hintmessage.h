/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#include <UtlVector.h>

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

class CHintMessage
{
public:
	CHintMessage(const char *hintString, bool isHint, CUtlVector<const char *> *args, duration_t duration);
	~CHintMessage(void);

public:
	duration_t GetDuration(void) const { return m_duration; }
	void Send(CBaseEntity *client);
	bool IsEquivalent(const char *hintString, CUtlVector<const char *> *args) const;

private:
	const char *m_hintString;
	CUtlVector<char *> m_args;
	duration_t m_duration;
	bool m_isHint;
};

class CHintMessageQueue
{
public:
	void Reset(void);
	void Update(CBaseEntity *player);
	bool AddMessage(const char *message, duration_t duration, bool isHint, CUtlVector<const char *> *args);
	bool IsEmpty(void) { return m_messages.Count() == 0; }

private:
	time_point_t m_tmMessageEnd;
	CUtlVector<CHintMessage *> m_messages;
};

}
