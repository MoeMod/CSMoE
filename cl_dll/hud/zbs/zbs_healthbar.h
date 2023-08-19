#pragma once

#include "hud_sub.h"
#include "r_texture.h"
#include <string>
#include <vector>

namespace cl {

	class CHudZBSHealthBar : public IBaseHudSub
	{
	public:
		CHudZBSHealthBar();

		int VidInit(void) override;
		void Reset(void) override;
		int Draw(float time) override;
		void Think(void) override;

		void UpdateHealth(float flPer, int iId);

	protected:
		UniqueTexture m_pTexture_BossHp[3];

		float m_pCurrentHealth;
		float m_pMaxHealth;
		float m_flPercentage;

		int m_iCurrentClassID;
		int m_iLastClassID;

	public:
		bool m_bShow;
	};


}