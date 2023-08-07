#ifndef ZBS_MAP_MANAGER_H
#define ZBS_MAP_MANAGER_H
#ifdef _WIN32
#pragma once
#endif

namespace cl {

	struct MessageData
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
	};


	void ZBS_LoadMessageData();
}

#endif