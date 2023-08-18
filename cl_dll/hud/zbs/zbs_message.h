
#ifndef ZBS_MAP_MANAGER_H
#define ZBS_MAP_MANAGER_H
#ifdef _WIN32
#pragma once
#endif

#include <zbs/zbs_msg.h>

namespace cl {

	struct ZBSMessageData
	{
		int id;
		std::string text;
		std::string image;
		std::string sound;
		int r;
		int g;
		int b;
		float fadeIn;
		float show;
		float fadeOut;
		int type;
		int textType;

		void operator=(CHudZBSMsg::ZbsMsgProperties_s& obj)
		{
			obj.pColor[0] = r;
			obj.pColor[1] = g;
			obj.pColor[2] = b;

			obj.szImage = image;
			obj.szSound = sound;
			obj.szText = text;

		};
		friend CHudZBSMsg;
	};


	void ZBS_LoadMessageData();
	
}

#endif