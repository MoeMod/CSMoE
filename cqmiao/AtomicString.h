//
// Created by 小白白 on 2019/12/30.
//

#ifndef SDP_SHARED_ATOMICSTRING_H
#define SDP_SHARED_ATOMICSTRING_H

#include <string>
#include <iostream>

#if 1
#include <atomic>
#include <memory>
// 23~24
class CAtomicString {
	std::shared_ptr<std::string> m_spStr;
public:
	CAtomicString(std::string str = {}) : m_spStr(std::make_shared<std::string>(std::move(str))) {}
	std::string load() const
	{
		auto sp = std::atomic_load(&m_spStr);
		return *sp;
	}
	void store(std::string what)
	{
		auto sp = std::make_shared<std::string>(std::move(what));
		std::atomic_store(&m_spStr, sp);
	}
};
#elif 0
// 9
#include <shared_mutex>

class CAtomicString {
	std::string str;
	std::shared_mutex m;
public:
	std::string load()
	{
		std::shared_lock<std::shared_mutex> lock(m);
		std::string ret = str;
		return ret;
	}
	void store(std::string what)
	{
		std::unique_lock<std::shared_mutex> lock(m);
		str = std::move(what);
	}
};
#elif 0
// 17~20
#include <mutex>

class CAtomicString {
	std::string str;
	std::mutex m;
public:
	std::string load()
	{
		std::lock_guard<std::mutex> lock(m);
		std::string ret = str;
		return ret;
	}
	void store(std::string what)
	{
		std::lock_guard<std::mutex> lock(m);
		str = std::move(what);
	}
};
#elif 0
#include <atomic>

class CAtomicString {
	std::atomic<char *> atomic_ptr;
public:
	std::string load()
	{
		// 防止这边在拷贝string的过程中被另一边删除，因此直接设置nullptr
		char *old_ptr = nullptr;
		// 只有取得非空指针后才能认为抢占到所有权
		while(old_ptr == nullptr) { old_ptr = atomic_ptr.exchange(nullptr); }
		assert(old_ptr != nullptr);
		// 当前线程进行拷贝
		std::string ret = old_ptr;
		// 归还先前的指针
		char *null_ptr = nullptr;
		auto result = atomic_ptr.compare_exchange_strong(null_ptr, old_ptr);
		assert(result == true && null_ptr == nullptr);
		return ret;
	}
	void store(std::string what)
	{
		char *new_ptr = new char[what.size() + 1];
		std::uninitialized_copy(what.begin(), what.end(), new_ptr);
		new_ptr[what.size()] = '\0';

		// 如果是空指针说明有人正在读取并拷贝，需要阻塞等待...
		char *old_ptr = nullptr;
		do
		{
			while(old_ptr == nullptr) { old_ptr = atomic_ptr.load(); }
		} while(!atomic_ptr.compare_exchange_weak(old_ptr, new_ptr));
		assert(old_ptr != nullptr);
		delete[] old_ptr;
	}
	CAtomicString() : atomic_ptr(nullptr) {
		atomic_ptr.store(new char[1]{});
	}
	~CAtomicString() {
		char *old_ptr = atomic_ptr.exchange(nullptr);
		delete[] old_ptr;
	}
};
#endif


inline std:: chrono::high_resolution_clock::duration TestAtomicString() {
	std::atomic<long> i = 0;
	CAtomicString as;
	auto t0 = std::chrono::high_resolution_clock::now();

	auto f1 = [&i, &as]{ while(i < 999999) as.store(std::to_string(i)); };
	auto f2 = [&i, &as]{ while(i < 999999) ++i, std::cout << as.load() << std::endl, std::this_thread::yield(); };

	std::vector<std::thread> vt1;
	std::generate_n(std::back_inserter(vt1), 10, [f1]{ return std::thread(f1); });
	std::vector<std::thread> vt2;
	std::generate_n(std::back_inserter(vt2), 10, [f2]{ return std::thread(f2); });

	std::for_each(vt1.begin(), vt1.end(), std::mem_fn(&std::thread::detach));
	std::for_each(vt2.begin(), vt2.end(), std::mem_fn(&std::thread::detach));

	while(i < 999999) std::this_thread::yield();

	auto t1 = std::chrono::high_resolution_clock::now();

	using namespace std::chrono_literals;
	std::this_thread::sleep_for(1s);

	return (t1 - t0);
}

#endif //SDP_SHARED_ATOMICSTRING_H