#pragma once

#include <vector>
#include <string>

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif
	struct zsht_buildmenu_s
	{
		int id;
		int submenu;
		int tier;
		int dependskill;
		int dependbuilding;
		std::string command;
		std::string Name;
		std::string Tooltip;
		std::string DisableTooltip;
		int wood;
		int iron;
		int power;
		bool useGlobalRes;
	};

	const std::vector<zsht_buildmenu_s>& ZombieShelterTeam_BuildMenuCfg();
}