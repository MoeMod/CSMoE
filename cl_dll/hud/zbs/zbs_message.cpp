#define RAPIDCSV	0

#include "zbs_message.h"

#include "../3rdparty/csv-parser/csv.hpp"

//#include <boost/iostreams/device/array.hpp>
//#include <boost/iostreams/stream.hpp>
#if  RAPIDCSV
#include "../3rdparty/rapidcsv/rapidcsv.h"
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

#if  RAPIDCSV
struct memorybuf : std::streambuf {

	memorybuf() = delete;
	memorybuf(const memorybuf&) = delete;
	memorybuf& operator=(const memorybuf&) = delete;

	memorybuf(const void* _Ptr, size_t _Count)
	{
		this->_Myfirst = (char*)_Ptr;
		this->_Mylast = this->_Myfirst + _Count;
		this->setg(_Myfirst, _Myfirst, _Mylast);
	}

	virtual pos_type __CLR_OR_THIS_CALL seekoff(off_type _Off, std::ios_base::seekdir _Way, std::ios_base::openmode _Mode) override
	{
		auto _Next = _Myfirst;
		switch (_Way) {
		case std::ios_base::beg:
			_Next = _Myfirst + _Off; break;
		case std::ios_base::cur:
			_Next = this->gptr() + _Off; break;
		case std::ios_base::end:
			_Next = _Mylast + _Off; break;
		}
		if (_Next >= _Myfirst && _Next <= _Mylast) {
			this->setg(_Myfirst, _Next, _Mylast);
		}

		return pos_type{ off_type{ this->gptr() - _Myfirst } };
	}


private:
	char* _Myfirst;
	char* _Mylast;

};

struct imemorystream : std::istream {

	imemorystream() = delete;
	imemorystream(const imemorystream&) = delete;
	imemorystream& operator=(const imemorystream&) = delete;

	imemorystream(const void* ptr, size_t length)
		: _Buf(ptr, length)
		, std::istream(&_Buf) { }

private:
	memorybuf _Buf;
};
#endif


namespace cl {

	std::map<int, ZBSMessageData> g_ZBSMessageData;

	void ZBS_LoadMessageData()
	{
		g_ZBSMessageData.clear();

		fs_offset_t length;
		auto data = gFileSystemAPI.FS_MapFile("resource/zbs_message.csv", &length, true);
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

					int col_id = csv.index_of("id");
					int col_text = csv.index_of("text");
					int col_image = csv.index_of("image");
					int col_sound = csv.index_of("sound");
					int col_r = csv.index_of("R");
					int col_g = csv.index_of("G");
					int col_b = csv.index_of("B");
					int col_fadeIn = csv.index_of("fadeIn");
					int col_show = csv.index_of("show");
					int col_fadeOut = csv.index_of("fadeOut");
					int col_type = csv.index_of("type");
					int col_textType = csv.index_of("textType");

					for (csv::CSVRow& row : csv)
					{
						auto val_id = csv_field_get<int> (row[col_id]);
						auto val_text = csv_field_get<std::string> (row[col_text]);
						auto val_image = csv_field_get<std::string>(row[col_image]);
						auto val_sound = csv_field_get<std::string>(row[col_sound]);
						auto val_r = csv_field_get<int>(row[col_r]);
						auto val_g = csv_field_get<int>(row[col_g]);
						auto val_b = csv_field_get<int>(row[col_b]);
						auto val_fadeIn = csv_field_get<float>(row[col_fadeIn]);
						auto val_show = csv_field_get<float>(row[col_show]);
						auto val_fadeOut = csv_field_get<float>(row[col_fadeOut]);
						auto val_type = csv_field_get<int>(row[col_type]);
						auto val_textType = csv_field_get<int>(row[col_textType]);

						g_ZBSMessageData.emplace(std::make_pair(val_id, ZBSMessageData{
							val_id,
							std::move(val_text),
							std::move(val_image),
							std::move(val_sound),
							val_r,
							val_g,
							val_b,
							val_fadeIn,
							val_show,
							val_fadeOut,
							val_type,
							val_textType
							}));

					}
#if RAPIDCSV
					imemorystream ms(data, length);
					rapidcsv::Document csv(ms);


					for (size_t i = 0; i < csv.GetRowCount(); i++)
					{
						std::vector<std::string> row = csv.GetRow<std::string>(i);

						if (row.size() < 12)
							continue;

						MessageData msgData;
						msgData.id = atoi(row[0].c_str());
						msgData.text = row[1].c_str();
						msgData.image = row[2].c_str();
						msgData.sound = row[3].c_str();
						msgData.r = atoi(row[4].c_str());
						msgData.g = atoi(row[5].c_str());
						msgData.b = atoi(row[6].c_str());
						msgData.fadeIn = atof(row[7].c_str());
						msgData.show = atof(row[8].c_str());
						msgData.fadeOut = atof(row[9].c_str());
						msgData.type = atoi(row[10].c_str());
						msgData.textType = atoi(row[11].c_str());

						g_ZBSMessageData.insert(std::pair<int, MessageData>(msgData.id, msgData));
					}
#endif
				}
			}
			gFileSystemAPI.FS_MapFree(data, length);
		}
	}
}

