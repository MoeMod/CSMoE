
#pragma once

#include "hud_sub.h"
#include "r_texture.h"
#include <string>
#include <vector>

namespace cl {

	class CHudZBSRank : public IBaseHudSub
	{
	public:
		CHudZBSRank();

		int VidInit(void) override;
		void Reset(void) override;
		int Draw(float time) override;
		void Think(void) override;

		void PushRankData(int iIndex, float flData);

		wchar_t* UTF8ToUnicode(const char* str)
		{
			static wchar_t result[256];
			Q_UTF8ToUnicode(str, result, 255);
			return result;
		}

	protected:
		SharedTexture m_pTexture_DamageBg;
		SharedTexture m_pTexture_DamageText;

		typedef struct ZbsRankProp_t
		{
			int iIndex;
			std::string szName;
			float flDamage;

		}ZbsRankProp_s;

		std::vector<std::pair<int, ZbsRankProp_s>> m_vecRankData;

		struct ArrangeData
		{
			bool operator()(const ZbsRankProp_s& Val1, const ZbsRankProp_s& Val2)
			{
				return Val1.flDamage > Val2.flDamage;
			}
		};


		float m_flTotalDamage;

		class ZbsRankDamage
		{
		public:
			ZbsRankDamage()
			{
				m_flTotalDamage = 0;
			}
			void operator()(const ZbsRankProp_s& Val)
			{
				m_flTotalDamage += Val.flDamage;
			}
			float GetTotalDamage() const
			{
				return m_flTotalDamage;
			}

		private:
			float m_flTotalDamage;
		};


	};


}