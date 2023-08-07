#include "hud.h"
#include "cl_util.h"

#include <string>
#include <sstream>

#include "zbs_message.h"
#include "../3rdparty/rapidcsv/rapidcsv.h"
namespace cl {

	std::map<int, MessageData> g_ZBSMessageData;

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
					if (memcmp(data, "\xEF\xBB\xBF", 3) == 0)
					{
						data = (char*)data + 3;
					}

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
				}
				gFileSystemAPI.FS_MapFree(data, length);
			}
		}
	}
}

