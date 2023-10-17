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


#include "weaponpaint.h"
#include "../3rdparty/csv-parser/csv.hpp"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

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

	std::vector<WeaponPaint> g_vecWeaponPaintList;

	static CWeaponPaintManager g_WeaponPaintManager;

	CWeaponPaintManager& WeaponPaintManager(void)
	{
		return g_WeaponPaintManager;
	}

	void CWeaponPaintManager::Init(void)
	{
		g_vecWeaponPaintList.clear();

		fs_offset_t length;
		auto data = gFileSystemAPI.FS_MapFile("resource/WeaponPaint/WeaponPaint_Texture.csv", &length, true);
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

					int col_modelName = csv.index_of("m_pStudioHeader.Name");
					int col_texID = csv.index_of("pmesh->skinref");
					int col_itemID = csv.index_of("ItemID");
					int col_replacedTex = csv.index_of("replacedTex");

					for (csv::CSVRow& row : csv)
					{

						auto val_modelName = csv_field_get<std::string>(row[col_modelName]);
						auto val_texID = csv_field_get<int>(row[col_texID]);
						auto val_itemID = csv_field_get<int>(row[col_itemID]);
						auto val_replacedTex = csv_field_get<std::string>(row[col_replacedTex]);

						g_vecWeaponPaintList.push_back(WeaponPaint{
							std::move(val_modelName),
							val_texID,
							val_itemID,
							std::move(val_replacedTex),
							});

					}

				}
			}
			gFileSystemAPI.FS_MapFree(data, length);
		}

	}

}
