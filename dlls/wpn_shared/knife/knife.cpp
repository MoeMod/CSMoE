#include "extdll.h"

#include "knife.h"

#include <map>
#include <memory>
#include <string>

#include "knife_default.h"

static std::map<KnifeID, std::unique_ptr<IKnifeHelper>> g_KnifeHelperCache;

static std::map<std::string, KnifeID> g_KnifeNameId{
	{"knife", KNIFE_NORMAL },
	{ "knife_zombi", KNIFE_ZOMBI_TANK }
};

IKnifeHelper *KnifeHelper_Get(KnifeID idx)
{
	auto iter = g_KnifeHelperCache.find(idx);
	if (iter != g_KnifeHelperCache.end())
		return iter->second.get();
	return nullptr;
}