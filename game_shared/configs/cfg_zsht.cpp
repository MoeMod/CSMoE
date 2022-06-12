#include "extdll.h"
#include "enginecallback.h"

#include "cfg_zsht.h"
#include "fs_int.h"

#include <sstream>
#include <string>

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
    extern fs_api_t gFileSystemAPI;
#endif
    const std::vector<zsht_buildmenu_s>& ZombieShelterTeam_BuildMenuCfg() {
        static std::vector<zsht_buildmenu_s> singleton;

        if (singleton.empty())
        {
            file_t* file = gFileSystemAPI.FS_Open("resource/shelterteam/zsht_buildmenu.csv", "rb", false);
            if (file)
            {
                auto readline = [](file_t* pFile) {
                    std::string ret;
                    char ch = '\0';
                    while (gFileSystemAPI.FS_Read(pFile, (void*)&ch, sizeof(char)))
                    {
                        if (ch == '\n')
                            return std::make_pair(true, ret);
                        ret.push_back(ch);
                    }
                    return std::make_pair(false, ret);
                };

                // skip first line in csv;
                readline(file);

                std::pair<bool, std::string> linedata;
                while ((linedata = readline(file)).first)
                {
                    std::istringstream ss(linedata.second);
                    std::string data;

                    zsht_buildmenu_s tempData;
                    std::getline(ss, data, ',');
                    tempData.id = atoi(data.c_str());
                    std::getline(ss, data, ',');
                    tempData.submenu = atoi(data.c_str());
                    std::getline(ss, data, ',');
                    tempData.tier = atoi(data.c_str());
                    std::getline(ss, data, ',');
                    tempData.dependskill = atoi(data.c_str());
                    std::getline(ss, data, ',');
                    tempData.dependbuilding = atoi(data.c_str());
                    std::getline(ss, data, ',');
                    tempData.command = data;
                    std::getline(ss, data, ',');
                    tempData.Name = data;
                    std::getline(ss, data, ',');
                    tempData.Tooltip = data;
                    std::getline(ss, data, ',');
                    tempData.DisableTooltip = data;
                    std::getline(ss, data, ',');
                    tempData.wood = atoi(data.c_str());
                    std::getline(ss, data, ',');
                    tempData.iron = atoi(data.c_str());
                    std::getline(ss, data, ',');
                    tempData.power = atoi(data.c_str());
                    std::getline(ss, data, ',');
                    tempData.useGlobalRes = atoi(data.c_str()) ? true : false;

                    singleton.emplace_back(tempData);
                }
                gFileSystemAPI.FS_Close(file);
            }
        }

        return singleton;
    }
}