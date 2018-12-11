#pragma once

#include <memory>

class CHostageImprov;
class CNpcGroup;
class CNpcGroupMember;

class CNpcGroup : public std::enable_shared_from_this<CNpcGroup>
{
	friend class CNpcGroupMember;

public:
	CNpcGroup(const Vector &origin);
	~CNpcGroup();
	std::shared_ptr<CNpcGroupMember> Join();
	bool IsCurrentAt(const Vector &vecWhere) const;
	//void Merge(CNpcGroup &&other);

private:
	struct impl_t;
	const std::unique_ptr<impl_t> pimpl;
};

class CNpcGroupMember
{
public:
	CNpcGroupMember(const std::shared_ptr<CNpcGroup> &g) : m_spGroup(g) {}
	const std::shared_ptr<const CNpcGroup> &GetGroup() const { return m_spGroup; }
	bool Apply(CHostageImprov *improv);

private:
	std::shared_ptr<const CNpcGroup> m_spGroup;
};

std::shared_ptr<CNpcGroupMember> GetGroup(const Vector &start);