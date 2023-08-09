
#pragma once

#include "hud_sub.h"
#include "r_texture.h"
#include <string>
#include <vector>

namespace cl {

	class CHudZBSMsg : public IBaseHudSub
	{
	public:
		CHudZBSMsg();

		int VidInit(void) override;
		void Reset(void) override;
		int Draw(float time) override;
		void Think(void) override;

		void PushMsg(int iID);

	protected:
		bool bInit;

		SharedTexture m_pTexture_TextBG_Left;
		SharedTexture m_pTexture_TextBG_Center;
		SharedTexture m_pTexture_TextBG_Right;

		UniqueTexture m_pTexture_Warning;

		typedef struct ZbsMsgProperties_t
		{
			int iId;
			std::string szText;
			std::string szImage;
			std::string szSound;
			Color pColor;

			bool operator==(const ZbsMsgProperties_t& obj) const
			{
				return obj.iId == iId;
			};
			

		}ZbsMsgProperties_s;

		typedef struct MsgDisplay_t
		{
			bool m_bPlaying;
			bool m_bReset;
			bool m_bBackGround;
			float m_flDisplayTime;
			float m_flFadeTime;

			std::vector <ZbsMsgProperties_s> m_vecMsg;
		}MsgDisplay_s;

		MsgDisplay_t m_MsgDisplay;


		bool Compare(const ZbsMsgProperties_s& x, int y)
		{
			return x.iId == y;
		}

		friend struct MessageData;

	};


}