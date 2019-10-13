#ifndef VGUI2_SRC_VGUIKEYVALUES_H
#define VGUI2_SRC_VGUIKEYVALUES_H

#include <tier1/MemPool.h>
#include <tier1/UtlRBTree.h>
#include <tier1/UtlVector.h>

#include <vgui/IKeyValues.h>

class CVGuiKeyValues : public IKeyValues
{
private:
	struct hash_item_t
	{
		int stringIndex;
		hash_item_t* next;
	};

	struct MemoryLeakTracker_t
	{
		int nameIndex;
		void* pMem;
	};

public:
	CVGuiKeyValues();
	~CVGuiKeyValues();

	void RegisterSizeofKeyValues( int size ) override;

	void *AllocKeyValuesMemory( int size ) override;
	void FreeKeyValuesMemory( void *pMem ) override;

	HKeySymbol GetSymbolForString( const char *name ) override;
	const char *GetStringForSymbol( HKeySymbol symbol ) override;

	void GetLocalizedFromANSI( const char* ansi, wchar_t* outBuf, int unicodeBufferSizeInBytes ) override;

	void GetANSIFromLocalized( const wchar_t* wchar, char* outBuf, int ansiBufferSizeInBytes ) override;

	void AddKeyValuesToMemoryLeakList( void *pMem, HKeySymbol name ) override;
	void RemoveKeyValuesFromMemoryLeakList( void *pMem ) override;

private:
	static bool MemoryLeakTrackerLessFunc( const MemoryLeakTracker_t& lhs, const MemoryLeakTracker_t& rhs );

	static unsigned int CaseInsensitiveHash( const char* string, int iBounds );

private:
	CMemoryPool* m_pMemPool = nullptr;

	int m_iMaxKeyValuesSize = 4;

	CUtlVector<char> m_Strings;
	CMemoryPool m_HashItemMemPool;

	CUtlVector<hash_item_t> m_HashTable;
	CUtlRBTree<MemoryLeakTracker_t, int> m_KeyValuesTrackingList;

private:
	CVGuiKeyValues( const CVGuiKeyValues& ) = delete;
	CVGuiKeyValues& operator=( const CVGuiKeyValues& ) = delete;
};

#endif //VGUI2_SRC_VGUIKEYVALUES_H
