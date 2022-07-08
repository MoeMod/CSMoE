#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"

#include "shared_util.h"
#include "UtlVector.h"

namespace sv {

CHintMessage::CHintMessage(const char *hintString, bool isHint, CUtlVector<const char *> *args, duration_t duration)
{
	m_hintString = hintString;
	m_duration = duration;
	m_isHint = isHint;

	if (args) {
		for (int i = 0; i < args->Count(); ++i)
			m_args.emplace_back((*args)[i]);
	}
}

void CHintMessage::Send(CBaseEntity *client)
{
	CUtlVector<char *> args;
	for(std::string &arg : m_args)
		args.AddToTail(arg.data());
	UTIL_ShowMessageArgs(m_hintString.c_str(), client, &args, m_isHint);
}

void CHintMessageQueue::Reset()
{
	m_tmMessageEnd = invalid_time_point;
	m_messages.clear();
}

void CHintMessageQueue::Update(CBaseEntity *client)
{
	if (gpGlobals->time <= m_tmMessageEnd)
		return;

	if (m_messages.empty())
		return;

	CHintMessage &msg = m_messages.front();
	m_tmMessageEnd = gpGlobals->time + msg.GetDuration();
	msg.Send(client);
	m_messages.erase(m_messages.begin());
}

bool CHintMessageQueue::AddMessage(const char *message, duration_t duration, bool isHint, CUtlVector<const char *> *args)
{
	m_messages.emplace_back(message, isHint, args, duration);

	return true;
}

}
