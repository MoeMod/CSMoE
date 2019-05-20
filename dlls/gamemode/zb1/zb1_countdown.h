/*
zb1_countdown.h - CSMoE Gameplay server
Copyright (C) 2019 Moemod Hymei

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef PROJECT_ZB1_COUNTDOWN_H
#define PROJECT_ZB1_COUNTDOWN_H

#include <memory>

namespace sv {

class IBaseMod;

class ICountdownDelegate
{
public:
	virtual ~ICountdownDelegate() = default;
	virtual void OnCountdownStart() = 0;
	virtual void OnCountdownChanged(int iCount) = 0;
	virtual void OnCountdownEnd() = 0;
};

class CModCountdownHelper
{
public:
	CModCountdownHelper(IBaseMod* mod, std::unique_ptr<ICountdownDelegate> m_pDelegate) : m_pDelegate(std::move(m_pDelegate)), m_pMod(mod), m_iTotalCounts(20), m_iLastCountDown(-1){}

	void Think();

	void SetCounts(int x) { m_iTotalCounts = x; }
	void SetDelegate(std::unique_ptr<ICountdownDelegate> p) { m_pDelegate = std::move(p); }

	bool IsExpired() const;
	bool IsFreezePeriod() const;

private:
	std::unique_ptr<ICountdownDelegate> m_pDelegate;
	IBaseMod* m_pMod;
	int m_iTotalCounts;
	int m_iLastCountDown;

};

}


#endif //PROJECT_ZBB_GHOST_H
