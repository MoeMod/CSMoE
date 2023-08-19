
#pragma once

#include "hud_sub.h"
#include "r_texture.h"
#include <string>
#include <vector>

namespace cl {

	extern inline char* va(const char* format, ...);
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
		float m_flTotalDamage;

		struct ArrangeData
		{
			int iIndex;

			void ParsingIndex(int index)
			{
				iIndex = index;
			}
			bool operator()(const std::pair<int, ZbsRankProp_s>& Val1, const std::pair<int, ZbsRankProp_s>& Val2)
			{
				return Val1.second.flDamage > Val2.second.flDamage;
			}
			bool operator()(const std::pair<int, ZbsRankProp_s> Val1)
			{
				return Val1.first == iIndex;
			}
		};

		struct ZbsRankDamage
		{

			ZbsRankDamage()
			{
				m_flTotalDamage = 0;
			}
			void operator()(const std::pair<int, ZbsRankProp_s>& Val)
			{
				m_flTotalDamage += Val.second.flDamage;
			}
			float GetTotalDamage() const
			{
				return m_flTotalDamage;
			}
			float m_flTotalDamage;
		};
	public:
		void DrawIntentionally(bool bCycle, int i, int& iX, int& iY, int& iW, int& iH, std::vector<std::pair<int, ZbsRankProp_s>>::iterator pFirst, int bIsLoacl);
		bool m_bShow;
	};


}