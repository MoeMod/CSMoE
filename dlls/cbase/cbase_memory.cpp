#include "extdll.h"
#include "util.h"
#include "cbase.h"

#include <algorithm>
#include <mutex>
#include <atomic>

static std::atomic<entvars_t *> g_pCachedEntVarsPtr = nullptr;
// operator new -> CBaseEntity()
static std::mutex g_CreateEntityLock;

static std::atomic<bool> g_bDontDestruct = false;
// ~CBaseEntity() -> operator delete
static std::mutex g_RemoveEntityLock;

// allocate memory for CBaseEntity with given pev
void *CBaseEntity::operator new(size_t stAllocateBlock, entvars_t *newpev)
{
	// nothing to do with nullptr
	if (newpev == nullptr)
		return nullptr;

	// cache pev
	g_CreateEntityLock.lock();
	g_pCachedEntVarsPtr.store(newpev);

	// alloc memory and then the constructor knows newpev...
	return ALLOC_PRIVATE(ENT(newpev), static_cast<long>(stAllocateBlock));
}

// No way to ban creating auto var anymore... 
// However, entity can only create by GetClassPtr<T>(nullptr), CREATE_NAMED_ENTITY,  or new T;
CBaseEntity::CBaseEntity() : pev(g_pCachedEntVarsPtr.exchange(nullptr))
{
	assert(pev != nullptr);
	assert(g_CreateEntityLock.try_lock() == false && "auto var of CBaseEntity is not allowed!");
	g_CreateEntityLock.unlock();
}

CBaseEntity::~CBaseEntity()
{
	g_RemoveEntityLock.lock();
	g_pCachedEntVarsPtr.store(this->pev);
}

void CBaseEntity::operator delete(void *pMem, entvars_t *pev)
{
	assert(pev != nullptr);

	edict_t *pEdict = ENT(pev);

	assert(pEdict->pvPrivateData == pMem);

	// FREE_PRIVATE may call OnFreeEntPrivateData, when the entity was never constructed, or already destructed...
	bool prev = g_bDontDestruct.exchange(true);
	//FREE_PRIVATE(pEdict);
	REMOVE_ENTITY(pEdict);
	g_bDontDestruct.store(prev);
}

void CBaseEntity::CheckEntityDestructor(CBaseEntity *pEntity)
{
	if (!g_bDontDestruct.load())
	{
		// call from engine to destruct
		pEntity->~CBaseEntity();
		assert(g_RemoveEntityLock.try_lock() == false);
		g_RemoveEntityLock.unlock();
	}
}

void *CBaseEntity::operator new(size_t stAllocateBlock)
{
	entvars_t *newpev = VARS(CREATE_ENTITY());
	void *result = operator new(stAllocateBlock, newpev);

	// failed? should free newpev
	if (result == nullptr)
		operator delete(nullptr, newpev);

	return result;
}

void CBaseEntity::operator delete(void *pMem)
{
	// nothing to do with nullptr
	if (pMem == nullptr)
		return;

	// free both the memory of CBaseEntity and cached pev (get from destructor)
	operator delete(pMem, g_pCachedEntVarsPtr.exchange(nullptr));
	assert(g_RemoveEntityLock.try_lock() == false);
	g_RemoveEntityLock.unlock();
}