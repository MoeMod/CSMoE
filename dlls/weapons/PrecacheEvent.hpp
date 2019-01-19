//
// Created by 小白白 on 2019-01-19.
//

template<class CFinal, class CBase = CBaseTemplateWeapon>
class TPrecacheEvent : public CBase
{
public:
	void Precache(void) override
	{
		CBase::Precache();
		auto &wpn = static_cast<CFinal &>(*this);
		m_usFire = PRECACHE_EVENT(1, wpn.EventFile);
	}

	unsigned short m_usFire;
};