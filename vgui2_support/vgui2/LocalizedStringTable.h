#ifndef VGUI2_SRC_LOCALIZEDSTRINGTABLE_H
#define VGUI2_SRC_LOCALIZEDSTRINGTABLE_H

#include <cstdarg>

#include <tier1/UtlRBTree.h>
#include <tier1/UtlSymbol.h>
#include <tier1/UtlVector.h>

#include <vgui/ILocalize.h>

class CLocalizedStringTable : vgui2::ILocalize
{
private:
	struct localizedstring_t
	{
		vgui2::StringIndex_t nameIndex;
		vgui2::StringIndex_t valueIndex;
		CUtlSymbol filename;
	};

	struct fastvalue_t
	{
		int valueindex;
		const wchar_t* search;
	};

public:
	CLocalizedStringTable();
	~CLocalizedStringTable();

	bool AddFile( IFileSystem *fileSystem, const char *fileName ) override;

	void RemoveAll() override;

	wchar_t *Find( char const *tokenName ) override;

	int ConvertANSIToUnicode( const char *ansi, wchar_t *unicode, int unicodeBufferSizeInBytes ) override;

	int ConvertUnicodeToANSI( const wchar_t *unicode, char *ansi, int ansiBufferSize ) override;

	vgui2::StringIndex_t FindIndex( const char *tokenName ) override;

	void ConstructString( wchar_t *unicodeOuput, int unicodeBufferSizeInBytes, wchar_t *formatString, int numFormatParameters, ... ) override;

	const char *GetNameByIndex( vgui2::StringIndex_t index ) override;
	wchar_t *GetValueByIndex( vgui2::StringIndex_t index ) override;

	vgui2::StringIndex_t GetFirstStringIndex() override;
	vgui2::StringIndex_t GetNextStringIndex( vgui2::StringIndex_t index ) override;

	void AddString( const char *tokenName, wchar_t *unicodeString, const char *fileName ) override;

	void SetValueByIndex( vgui2::StringIndex_t index, wchar_t *newValue ) override;

	bool SaveToFile( IFileSystem *fileSystem, const char *fileName ) override;

	int GetLocalizationFileCount() override;
	const char *GetLocalizationFileName( int index ) override;

	const char *GetFileNameByIndex( vgui2::StringIndex_t index ) override;

	void ReloadLocalizationFiles( /*IFileSystem *filesystem*/ ) override;

	void ConstructString( wchar_t *unicodeOutput, int unicodeBufferSizeInBytes, const char *tokenName, KeyValues *localizationVariables ) override;
	void ConstructString( wchar_t *unicodeOutput, int unicodeBufferSizeInBytes, vgui2::StringIndex_t unlocalizedTextSymbol, KeyValues *localizationVariables ) override;

	void ConstructString( wchar_t *unicodeOuput, int unicodeBufferSizeInBytes, wchar_t *formatString, int numFormatParameters, va_list argList );

private:
	static bool FastValueLessFunc( const fastvalue_t& lhs, const fastvalue_t& rhs );
	static bool SymLess( const localizedstring_t& lhs, const localizedstring_t& rhs );

	int FindExistingValueIndex( const wchar_t* value );

	bool AddAllLanguageFiles( IFileSystem* fileSystem, const char* baseFileName );

	void BuildFastValueLookup();

private:
	static CLocalizedStringTable* s_pTable;

	char m_szLanguage[ 64 ];
	bool m_bUseOnlyLongestLanguageString = false;

	CUtlRBTree<localizedstring_t, unsigned long> m_Lookup;
	
	CUtlVector<char> m_Names;
	CUtlVector<wchar_t> m_Values;

	CUtlSymbol m_CurrentFile;

	CUtlVector<CUtlSymbol> m_LocalizationFiles;
	CUtlRBTree<fastvalue_t, int> m_FastValueLookup;

private:
	CLocalizedStringTable( const CLocalizedStringTable& ) = delete;
	CLocalizedStringTable& operator=( const CLocalizedStringTable& ) = delete;
};

namespace vgui2
{
//Provided for backwards compatibility only.
//TODO: figure out if this interface is backwards compatible with 003. If so, remove this and expose 003 as both.
class CLocalizedStringtable_VGUI_Localize002 : public IBaseInterface
{
public:
	CLocalizedStringtable_VGUI_Localize002() = default;
	~CLocalizedStringtable_VGUI_Localize002() = default;

	virtual bool AddFile( IFileSystem *fileSystem, const char *fileName );

	virtual void RemoveAll();

	virtual wchar_t *Find( char const *tokenName );

	virtual int ConvertANSIToUnicode( const char *ansi, wchar_t *unicode, int unicodeBufferSizeInBytes );

	virtual int ConvertUnicodeToANSI( const wchar_t *unicode, char *ansi, int ansiBufferSize );

	virtual vgui2::StringIndex_t FindIndex( const char *tokenName );

	virtual void ConstructString( wchar_t *unicodeOuput, int unicodeBufferSizeInBytes, wchar_t *formatString, int numFormatParameters, ... );

	virtual const char *GetNameByIndex( vgui2::StringIndex_t index );
	virtual wchar_t *GetValueByIndex( vgui2::StringIndex_t index );

	virtual vgui2::StringIndex_t GetFirstStringIndex();
	virtual vgui2::StringIndex_t GetNextStringIndex( vgui2::StringIndex_t index );

	virtual void AddString( const char *tokenName, wchar_t *unicodeString, const char *fileName );

	virtual void SetValueByIndex( vgui2::StringIndex_t index, wchar_t *newValue );

	virtual bool SaveToFile( IFileSystem *fileSystem, const char *fileName );

	virtual int GetLocalizationFileCount();
	virtual const char *GetLocalizationFileName( int index );

	virtual const char *GetFileNameByIndex( vgui2::StringIndex_t index );

private:
	CLocalizedStringtable_VGUI_Localize002( const CLocalizedStringtable_VGUI_Localize002& ) = delete;
	CLocalizedStringtable_VGUI_Localize002& operator=( const CLocalizedStringtable_VGUI_Localize002& ) = delete;
};
}

#endif //VGUI2_SRC_LOCALIZEDSTRINGTABLE_H
