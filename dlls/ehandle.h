/*
*
*   This program is free software; you can redistribute it and/or modify it
*   under the terms of the GNU General Public License as published by the
*   Free Software Foundation; either version 2 of the License, or (at
*   your option) any later version.
*
*   This program is distributed in the hope that it will be useful, but
*   WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*   General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software Foundation,
*   Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*   In addition, as a special exception, the author gives permission to
*   link the code of this program with the Half-Life Game Engine ("HL
*   Engine") and Modified Game Libraries ("MODs") developed by Valve,
*   L.L.C ("Valve").  You must obey the GNU General Public License in all
*   respects for all of the code used other than the HL Engine and MODs
*   from Valve.  If you modify this file, you may extend this exception
*   to your version of the file, but you are not obligated to do so.  If
*   you do not wish to do so, delete this exception statement from your
*   version.
*
*/

#pragma once

// Safe way to point to CBaseEntities who may die between frames.
template <typename T = CBaseEntity>
class EntityHandle
{
public:
	// default constructor
	constexpr EntityHandle() : m_edict(nullptr), m_serialnumber(0) {}

	// copy constructor
	// Note : template functions cannot be  copy constructor
	constexpr EntityHandle(const EntityHandle<T> &other) : m_edict(other.m_edict), m_serialnumber(other.m_serialnumber) {}

	// Note : template functions are considered behind non-template functions
	template<class U, class = typename std::enable_if<std::is_base_of<T, U>::value>::type>
	constexpr EntityHandle(const EntityHandle<U> &other) : m_edict(other.m_edict), m_serialnumber(other.m_serialnumber) {}

	EntityHandle(const T *pEntity);
	explicit EntityHandle(edict_t *pEdict);

	template<class U>
	friend class EntityHandle;

	// cast to base class
	// NOTE: this is a unsafe method
	template <typename R>
	auto Get() const -> typename std::enable_if<std::is_base_of<R, T>::value, R *>::type
	{
		return GET_PRIVATE<R>(Get());
	}

	constexpr edict_t *Get() const;
	edict_t *Set(edict_t *pEdict);

	void Remove();
	bool IsValid() const;
	constexpr int GetSerialNumber() const;

	bool operator==(T *pEntity) const;
	friend bool operator==(T *pEntity, const EntityHandle<T> &rhs) { return rhs == pEntity; }

	// if(p), and also works for comparsion with NULL
	operator bool() const { return IsValid(); }
	// Gets the Entity this handle refers to.
	// Returns null if invalid.
	// works for comparsion with nullptr
	operator T *() const { return Get<T>(); }
	// Note : template functions are considered behind non-template functions
	template <class R, class = typename std::enable_if<std::is_base_of<R, T>::value, R*>::type>
	operator R *() const { return Get<R>(); }

	T *operator->() const;

private:
	edict_t *m_edict;
	int m_serialnumber;
};

// Short alias
using EHandle = EntityHandle<>;
using EHANDLE = EHandle;

// Inlines
template <typename T>
inline bool FNullEnt(const EntityHandle<T> &hent)
{
	return (!hent || FNullEnt(OFFSET(hent.Get())));
}

template <typename T>
EntityHandle<T>::EntityHandle(const T *pEntity) : EntityHandle()
{
	if (pEntity)
	{
		Set(ENT(pEntity->pev));
	}
}

template <typename T>
EntityHandle<T>::EntityHandle(edict_t *pEdict)
{
	Set((pEdict));
}

template <typename T>
constexpr inline edict_t *EntityHandle<T>::Get() const
{
	return (!m_edict || m_edict->serialnumber != m_serialnumber || m_edict->free) ? nullptr : m_edict;
}

template <typename T>
inline edict_t *EntityHandle<T>::Set(edict_t *pEdict)
{
	m_edict = pEdict;
	if (pEdict)
	{
		m_serialnumber = pEdict->serialnumber;
	}

	return pEdict;
}

template <typename T>
void EntityHandle<T>::Remove()
{
	if (IsValid())
	{
		UTIL_Remove(*this);
	}

	m_edict = nullptr;
	m_serialnumber = 0;
}

// Returns whether this handle is valid.
template <typename T>
inline bool EntityHandle<T>::IsValid() const
{
	edict_t *pEdict = Get();
	if (!pEdict)
	{
		return false;
	}

	CBaseEntity *pEntity = GET_PRIVATE<CBaseEntity>(pEdict);
	if (!pEntity)
	{
		return false;
	}

	return true;
}

// CBaseEntity serial number.
// Used to determine if the entity is still valid.
template <typename T>
constexpr inline int EntityHandle<T>::GetSerialNumber() const
{
	return m_serialnumber;
}

template <typename T>
inline bool EntityHandle<T>::operator==(T *pEntity) const
{
	//assert("EntityHandle<T>::operator==:  got a nullptr pointer!" && (pEntity != nullptr));
	if (pEntity == nullptr)
		return !IsValid();

	if (m_serialnumber != pEntity->edict()->serialnumber)
		return false;

	return m_edict == pEntity->edict();
}

template <typename T>
inline T *EntityHandle<T>::operator->() const
{
	edict_t *pEdict = Get();
	assert("EntityHandle<T>::operator->:  pointer is nullptr!" && (pEdict != nullptr));

	T *pEntity = GET_PRIVATE<T>(pEdict);
	assert("EntityHandle<T>::operator->:  pvPrivateData is nullptr!" && (pEntity != nullptr));
	return pEntity;
}