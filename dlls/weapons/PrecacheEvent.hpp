//
// Created by 小白白 on 2019-01-19.
//

template<class CFinal, class CBase = CBaseTemplateWeapon>
class TPrecacheEvent : public CBase
{
public:
	void Precache(void) override
	{
		auto &wpn = static_cast<CFinal &>(*this);
		m_usFire = PRECACHE_EVENT(1, wpn.EventFile);
		CBase::Precache();
	}

	unsigned short m_usFire;
};