#pragma once

#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <stdarg.h> // va
#ifdef _WIN32
#include <io.h>
#include <direct.h>
#ifdef XASH_FS_WIN32
#include <fileapi.h>
#else
#include "mman.h"
#endif
#else
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#endif

namespace xe {
	struct fs_posix {
		using fd_t = int;
		using off_t = ::off_t;
		static constexpr fd_t invalid_file = -1;

		static fd_t fopen(const char* path, const char* mode)
		{
			int flag = mode_to_flag(mode);
			return open(path, flag, 0777);
		}

		static int fclose(fd_t fd)
		{
			return close(fd);
		}

		static off_t fseek(fd_t fd, off_t off, int wherence)
		{
			return lseek(fd, off, wherence);
		}

		static size_t fread(fd_t fd, void* data, size_t count)
		{
			return ::read(fd, data, count);
		}

		static size_t fwrite(fd_t fd, const void* data, size_t count)
		{
			return ::write(fd, data, count);
		}

		static fd_t fdup(fd_t fd)
		{
			return ::dup(fd);
		}

        static void *mmap_file(fd_t fd, size_t len, off_t offset = 0, void *map_hint = nullptr)
        {
            return check_ptr(::mmap(map_hint, len, PROT_READ, MAP_FILE | MAP_SHARED, fd, offset));
        }

		static void* mmap_file_cow(fd_t fd, size_t len, off_t offset = 0, void* map_hint = nullptr)
		{
#ifdef _WIN32
			// plain impl in win32
			auto p = mmap_file(fd, len, offset, nullptr);
			auto p2 = mmap_anon(len, map_hint);
			memcpy(p2, p, len);
			munmap(p, len);
			return check_ptr(p2);
#else
			return check_ptr(::mmap(map_hint, len, PROT_READ | PROT_WRITE, MAP_FILE | MAP_PRIVATE, fd, offset));
#endif
		}

        static void *mmap_anon(size_t len, void *map_hint = nullptr)
        {
            return check_ptr(::mmap(map_hint, len, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0));
        }

        static void *check_ptr(void *p)
        {
            if(p == MAP_FAILED)
                return nullptr;
            return p;
        }

        static int munmap(void *addr, size_t len)
        {
            return addr ? ::munmap(addr, len) : 0;
        }

		static int mkdir(const char* path)
		{
			return ::_mkdir(path);
		}

        static off_t get_fd_size(fd_t fd)
        {
            off_t cur = ::lseek(fd, 0, SEEK_CUR);
            off_t end = ::lseek(fd, 0, SEEK_END);
            ::lseek(fd, cur, SEEK_SET);
            return end;
        }

		static int mode_to_flag(const char* mode)
		{
			int	mod = 0, opt = 0;
			switch (mode[0])
			{
			case 'r':
				mod = O_RDONLY;
				opt = 0;
				break;
			case 'w':
				mod = O_WRONLY;
				opt = O_CREAT | O_TRUNC;
				break;
			case 'a':
				mod = O_WRONLY;
				opt = O_CREAT | O_APPEND;
				break;
			}

			for (int ind = 1; mode[ind] != '\0'; ind++)
			{
				switch (mode[ind])
				{
				case '+':
					mod = O_RDWR;
					break;
				case 'b':
					opt |= O_BINARY;
					break;
				}
			}
			return mod | opt;
		}
	};

	struct fs_stdio
	{
		using fd_t = ::FILE *;
		using off_t = long;
		static constexpr fd_t invalid_file = nullptr;

		static fd_t fopen(const char* path, const char* mode)
		{
			return ::fopen(path, mode);
		}

		static int fclose(fd_t fd)
		{
			return ::fclose(fd);
		}

		static off_t fseek(fd_t fd, off_t off, int wherence)
		{
			return ::fseek(fd, off, wherence);
		}

		static size_t fread(fd_t fd, void* data, size_t count)
		{
			return ::fread(data, count, 1, fd);
		}

		static size_t fwrite(fd_t fd, const void* data, size_t count)
		{
			return ::fwrite(data, count, 1, fd);
		}

		static fd_t fdup(fd_t fd) = delete; // unsupported
	};
#ifdef _WIN32 // TODO : buggy
	struct fs_win32
	{
		using fd_t = ::HANDLE;
		using off_t = ::LONG;
		static constexpr fd_t invalid_file = INVALID_HANDLE_VALUE;

		static int mode_to_access(const char* mode)
		{
			switch (mode[0])
			{
			case 'r':
				return GENERIC_READ;
			case 'w':
			case 'a':
				if (strchr(mode, '+'))
					return GENERIC_READ | GENERIC_WRITE;
				else
					return GENERIC_WRITE;
			}

			return 0;
		}
		static int mode_to_share(const char* mode)
		{
			switch (mode[0])
			{
			case 'r':
				return FILE_SHARE_READ;
			case 'w':
			case 'a':
				if(strchr(mode, '+'))
					return FILE_SHARE_READ | FILE_SHARE_WRITE;
				else
					return FILE_SHARE_WRITE;
			}

			return 0;
		}
		static int mode_to_create(const char* mode)
		{
			int	opt = 0;
			switch (mode[0])
			{
			case 'r':
				opt = OPEN_EXISTING;
				break;
			case 'w':
				opt = CREATE_ALWAYS;
				break;
			case 'a':
				opt = OPEN_ALWAYS;
				break;
			}
			return opt;
		}
		static int mode_to_attr(const char* mode)
		{
			if (mode[0] == 'r')
				return FILE_ATTRIBUTE_READONLY;
			return FILE_ATTRIBUTE_NORMAL;
		}

		static int wherence_to_move_method(int wherence)
		{
			switch (wherence)
			{
			case SEEK_SET:
				return FILE_BEGIN;
			case SEEK_CUR:
				return FILE_CURRENT;
			case SEEK_END:
				return FILE_END;
			}
		}

		static fd_t fopen(const char* path, const char* mode, int pmode = 0)
		{
			// MoeMod : fix Chinese path
			wchar_t wpath[MAX_PATH];
			MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath, MAX_PATH);
			
			auto fd = ::CreateFileW(wpath, mode_to_access(mode), mode_to_share(mode), NULL, mode_to_create(mode), mode_to_attr(mode), NULL);
			if (strchr(mode, '+'))
				fseek(fd, 0, SEEK_END);
			else
				fseek(fd, 0, SEEK_SET);
			return fd;
		}

		static int fclose(fd_t fd)
		{
			return ::CloseHandle(fd);
		}

		static off_t fseek(fd_t fd, off_t off, int wherence)
		{
			return ::SetFilePointer(fd, off, NULL, wherence_to_move_method(wherence));
		}

		static size_t fread(fd_t fd, void* data, size_t count)
		{
			DWORD result = 0;
			::ReadFile(data, data, count, &result, NULL);
			return result;
		}

		static size_t fwrite(fd_t fd, const void* data, size_t count)
		{
			DWORD result = 0;
			::WriteFile(fd, data, count, &result, NULL);
			return result;
		}

		static fd_t fdup(fd_t fd)
		{
			fd_t result = invalid_file;
			::DuplicateHandle(::GetCurrentProcess(), fd, ::GetCurrentProcess(), &result, 0, FALSE, DUPLICATE_SAME_ACCESS);
			return result;
		}

		static int mkdir(const char* path)
		{
			// MoeMod : fix Chinese path
			wchar_t wpath[MAX_PATH];
			MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath, MAX_PATH);

			return ::CreateDirectoryW(wpath, NULL);
		}
	};
#endif

#ifdef XASH_FS_WIN32
	using fs = fs_win32;
#else
	using fs = fs_posix;
#endif
}