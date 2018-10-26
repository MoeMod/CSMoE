#include "hud.h"
#include "followicon.h"
#include "cl_util.h"
#include "draw_util.h"
#include "triangleapi.h"

#include "parsemsg.h"

#include "zb2.h"
#include "zb2_skill.h"

#include "gamemode/zb2/zb2_const.h"

#include <vector>

class CHudZB2::impl_t
{
public:
	CHudZB2_Skill skill;

	SharedTexture m_pTexture_RageRetina;
	std::vector<CHudRetina::MagicNumber> m_RetinaIndexes;
public:
	template<class T, class F, class...Args>
	void for_each(F T::*f, Args &&...args)
	{
		// add dispatch here.
		(skill.*f)(std::forward<Args>(args)...);
	}
};

DECLARE_MESSAGE(m_ZB2, ZB2Msg)
int CHudZB2::MsgFunc_ZB2Msg(const char *pszName, int iSize, void *pbuf)
{
	BufferReader buf(pszName, pbuf, iSize);
	ZB2MessageType type = static_cast<ZB2MessageType>(buf.ReadByte());
	switch (type)
	{
	case ZB2_MESSAGE_HEALTH_RECOVERY:
	{
		pimpl->skill.OnHealthRecovery();
		break;
	}
	case ZB2_MESSAGE_SKILL_INIT:
	{
		ZombieClassType zclass = static_cast<ZombieClassType>(buf.ReadByte());
		ZombieSkillType skills[4]{};
		for (int i = 0; i < 4 && !buf.Eof(); ++i)
			skills[i] = static_cast<ZombieSkillType>(buf.ReadByte());
		pimpl->skill.OnSkillInit(zclass, skills[0], skills[1], skills[2], skills[3]);

		// remove retinas...
		for (auto x : pimpl->m_RetinaIndexes)
		{
			gHUD.m_Retina.RemoveItem(x);
		}
		pimpl->m_RetinaIndexes.clear();
		break;
	}
	case ZB2_MESSAGE_SKILL_ACTIVATE:
	{
		ZombieSkillType type = static_cast<ZombieSkillType>(buf.ReadByte());
		float flHoldTime = buf.ReadShort();
		float flFreezeTime = buf.ReadShort();
		pimpl->skill.OnSkillActivate(type, flHoldTime, flFreezeTime);
		if (type == ZOMBIE_SKILL_CRAZY || type == ZOMBIE_SKILL_CRAZY2)
			pimpl->m_RetinaIndexes.push_back(gHUD.m_Retina.AddItem(pimpl->m_pTexture_RageRetina, CHudRetina::RETINA_DRAW_TYPE_BLINK, 10.0f));
		break;
	}
		
		
	}
	
	return 1;
}

int CHudZB2::Init(void)
{
	pimpl = new CHudZB2::impl_t;

	gHUD.AddHudElem(this);

	HOOK_MESSAGE(ZB2Msg);

	return 1;
}

int CHudZB2::VidInit(void)
{
	pimpl->for_each(&IBaseHudSub::VidInit);

	if(!pimpl->m_pTexture_RageRetina)
		pimpl->m_pTexture_RageRetina = R_LoadTextureShared("resource/zombi/zombicrazy");
	return 1;
}

int CHudZB2::Draw(float time)
{
	pimpl->for_each(&IBaseHudSub::Draw, time);
	return 1;
}

void CHudZB2::Think(void)
{
	pimpl->for_each(&IBaseHudSub::Think);
}

void CHudZB2::Reset(void)
{
	pimpl->for_each(&IBaseHudSub::Reset);
}

void CHudZB2::InitHUDData(void)
{
	pimpl->for_each(&IBaseHudSub::InitHUDData);
}

void CHudZB2::Shutdown(void)
{
	delete pimpl;
	pimpl = nullptr;
}