#pragma once

#include <atlcoll.h>
#include <atlstr.h>

class CFileVersionInfo
{
public:
	BOOL LoadFromFile(LPCTSTR lpName);
	BOOL QueryString(LPCTSTR lpSubBlock, CAtlString &value) const;
	BOOL GetFileVersion(LPDWORD pdwVer) const;
protected:
	CAtlArray<BYTE> m_VerInfoData;
	CAtlString m_VarQueryPrefix;
};
