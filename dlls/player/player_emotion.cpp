/*
gl_weaponpaint.h
Copyright (C) 2023 Moemod Hymei

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/


#include "player_emotion.h"
#include "../3rdparty/csv-parser/csv.hpp"

#include "FileSystem.h"
#include "fs_int.h"
#include "player_const.h"

namespace sv {

	template <class T>
	inline T csv_field_get(csv::CSVField&& field) {
		if (field.is_null()) {
			return { };
		}
		else {
			return field.get<T>();
		}
	}

	extern fs_api_t gFileSystemAPI;

	std::map<ModelName, PlayerEmotion> g_vecPlayerEmotionList;

	static CPlayerEmotionManager g_PlayerEmotionManager;

	CPlayerEmotionManager& PlayerEmotionManager(void)
	{
		return g_PlayerEmotionManager;
	}

	void CPlayerEmotionManager::Init(void)
	{
		g_vecPlayerEmotionList.clear();

		fs_offset_t length;
		auto data = gFileSystemAPI.FS_MapFile("classes2/emotion.csv", &length, true);
		if (data)
		{
			if (length)
			{
				if (length >= 3)
				{
					//FS_MapFree 应该传入FS_MapFile返回的data
					auto copydata = (char*)data;
					if (memcmp(data, "\xEF\xBB\xBF", 3) == 0)
					{
						copydata = (char*)copydata + 3;
					}

					auto len = strlen((char*)copydata);

					csv::CSVReader csv = csv::parse(std::string_view((const char*)copydata, len));

					int col_modelID = csv.index_of("model_id");
					int col_radioText = csv.index_of("radio_text");
					int col_emtionCount = csv.index_of("emtion_count");
					int col_wa01 = csv.index_of("wa01");
					int col_wa02 = csv.index_of("wa02");
					int col_wa03 = csv.index_of("wa03");
					int col_wa04 = csv.index_of("wa04");
					int col_wa05 = csv.index_of("wa05");
					int col_wa06 = csv.index_of("wa06");
					int col_wa07 = csv.index_of("wa07");
					int col_wa08 = csv.index_of("wa08");
					int col_wa09 = csv.index_of("wa09");
					int col_wa10 = csv.index_of("wa10");
					int col_seq01 = csv.index_of("seq01");
					int col_seq02 = csv.index_of("seq02");
					int col_seq03 = csv.index_of("seq03");
					int col_seq04 = csv.index_of("seq04");
					int col_seq05 = csv.index_of("seq05");
					int col_seq06 = csv.index_of("seq06");
					int col_seq07 = csv.index_of("seq07");
					int col_seq08 = csv.index_of("seq08");
					int col_seq09 = csv.index_of("seq09");
					int col_seq10 = csv.index_of("seq10");
					int col_frame01 = csv.index_of("frame01");
					int col_frame02 = csv.index_of("frame02");
					int col_frame03 = csv.index_of("frame03");
					int col_frame04 = csv.index_of("frame04");
					int col_frame05 = csv.index_of("frame05");
					int col_frame06 = csv.index_of("frame06");
					int col_frame07 = csv.index_of("frame07");
					int col_frame08 = csv.index_of("frame08");
					int col_frame09 = csv.index_of("frame09");
					int col_frame10 = csv.index_of("frame10");

					for (csv::CSVRow& row : csv)
					{
						ModelName val_modelID = ModelName(csv_field_get<int>(row[col_modelID]));
						auto val_radioText = csv_field_get<std::string>(row[col_radioText]);
						auto val_emtionCount = csv_field_get<int>(row[col_emtionCount]);
						auto val_wa01 = csv_field_get<int>(row[col_wa01]);
						auto val_wa02 = csv_field_get<int>(row[col_wa02]);
						auto val_wa03 = csv_field_get<int>(row[col_wa03]);
						auto val_wa04 = csv_field_get<int>(row[col_wa04]);
						auto val_wa05 = csv_field_get<int>(row[col_wa05]);
						auto val_wa06 = csv_field_get<int>(row[col_wa06]);
						auto val_wa07 = csv_field_get<int>(row[col_wa07]);
						auto val_wa08 = csv_field_get<int>(row[col_wa08]);
						auto val_wa09 = csv_field_get<int>(row[col_wa09]);
						auto val_wa10 = csv_field_get<int>(row[col_wa10]);
						auto val_seq01 = csv_field_get<std::string>(row[col_seq01]);
						auto val_seq02 = csv_field_get<std::string>(row[col_seq02]);
						auto val_seq03 = csv_field_get<std::string>(row[col_seq03]);
						auto val_seq04 = csv_field_get<std::string>(row[col_seq04]);
						auto val_seq05 = csv_field_get<std::string>(row[col_seq05]);
						auto val_seq06 = csv_field_get<std::string>(row[col_seq06]);
						auto val_seq07 = csv_field_get<std::string>(row[col_seq07]);
						auto val_seq08 = csv_field_get<std::string>(row[col_seq08]);
						auto val_seq09 = csv_field_get<std::string>(row[col_seq09]);
						auto val_seq10 = csv_field_get<std::string>(row[col_seq10]);
						auto val_frame01 = csv_field_get<int>(row[col_frame01]);
						auto val_frame02 = csv_field_get<int>(row[col_frame02]);
						auto val_frame03 = csv_field_get<int>(row[col_frame03]);
						auto val_frame04 = csv_field_get<int>(row[col_frame04]);
						auto val_frame05 = csv_field_get<int>(row[col_frame05]);
						auto val_frame06 = csv_field_get<int>(row[col_frame06]);
						auto val_frame07 = csv_field_get<int>(row[col_frame07]);
						auto val_frame08 = csv_field_get<int>(row[col_frame08]);
						auto val_frame09 = csv_field_get<int>(row[col_frame09]);
						auto val_frame10 = csv_field_get<int>(row[col_frame10]);
						

						g_vecPlayerEmotionList.emplace(std::make_pair(val_modelID, PlayerEmotion{
							val_modelID,
							std::move(val_radioText),
							val_emtionCount,
							val_wa01,
							val_wa02,
							val_wa03,
							val_wa04,
							val_wa05,
							val_wa06,
							val_wa07,
							val_wa08,
							val_wa09,
							val_wa10,
							val_seq01,
							val_seq02,
							val_seq03,
							val_seq04,
							val_seq05,
							val_seq06,
							val_seq07,
							val_seq08,
							val_seq09,
							val_seq10,
							val_frame01,
							val_frame02,
							val_frame03,
							val_frame04,
							val_frame05,
							val_frame06,
							val_frame07,
							val_frame08,
							val_frame09,
							val_frame10,
							}));

					}

				}
			}
			gFileSystemAPI.FS_MapFree(data, length);
		}

	}

}
