#include "stdafx.h"
#include "VersionInfo.h"

BOOL CFileVersionInfo::LoadFromFile(LPCTSTR lpName)
{
	DWORD dwSize = GetFileVersionInfoSize(lpName,NULL);
	if (!dwSize) return FALSE;

	if (!m_VerInfoData.SetCount(dwSize))
	{
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return FALSE;
	}

	if (!GetFileVersionInfo(lpName,0,dwSize,m_VerInfoData.GetData()))
		return FALSE;

	
	LPWORD pTranslation;
	UINT uTranslationSize;

	if (!VerQueryValue(m_VerInfoData.GetData(),_T("\\VarFileInfo\\Translation"),(LPVOID*)&pTranslation,&uTranslationSize))
		return FALSE;

	if (uTranslationSize<4)
	{
		SetLastError(ERROR_INVALID_DATA);
		return FALSE;
	}

	m_VarQueryPrefix.Format(_T("\\StringFileInfo\\%04X%04X\\"),pTranslation[0],pTranslation[1]);

	return TRUE;
}

BOOL CFileVersionInfo::QueryString(LPCTSTR lpSubBlock,CAtlString &value) const
{
	CAtlString SubBlock;

	LPCWSTR pData;
	UINT uData;

	SubBlock = m_VarQueryPrefix + lpSubBlock;

	if (!VerQueryValue(m_VerInfoData.GetData(),SubBlock,(LPVOID*)&pData,&uData))
		return FALSE;

	value = CAtlStringW(pData);
	return TRUE;
}

BOOL CFileVersionInfo::GetFileVersion(LPDWORD pdwVer) const
{
	CAtlString str;

	if (!QueryString(_T("FileVersion"),str))
		return FALSE;

	UINT v1,v2,v3,v4;
	if (_stscanf_s(str,_T("%u.%u.%u.%u"),&v1,&v2,&v3,&v4)!=4)
	{
		SetLastError(ERROR_INVALID_DATA);
		return FALSE;
	}

	*pdwVer = (BYTE)v1 << 24 | (BYTE)v2 << 16  | (BYTE)v3 << 8 | (BYTE)v4;
	return TRUE;
}

