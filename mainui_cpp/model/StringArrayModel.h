#pragma once
#ifndef STRINGARRAYMODEL_H
#define STRINGARRAYMODEL_H

#include "BaseArrayModel.h"
namespace ui {
class CStringArrayModel : public CMenuBaseArrayModel
{
public:
	// pointer to array of pointers
	CStringArrayModel( const char **ptr, int count )
	{
		m_u.m_pArrayOfPtrs = ptr;
		m_iCount = count;
		m_iOffset = 0;
	}

	CStringArrayModel( const char *str, int offset, int count )
	{
		m_u.m_pArrayOfChars = str;
		m_iOffset = offset;
		m_iCount = count;
	}

	// by default, there is no need to update
	void Update() { }

	const char *GetText( int line )
	{
		if( m_iOffset )
			return m_u.m_pArrayOfChars + m_iOffset * line;
		return m_u.m_pArrayOfPtrs[line];
	}

	int GetRows() const
	{
		return m_iCount;
	}

protected:
	int m_iCount;

private:
	union
	{
		const char **m_pArrayOfPtrs;
		const char *m_pArrayOfChars;
	} m_u;
	int m_iOffset;
};
}
#endif // STRINGARRAYMODEL_H
