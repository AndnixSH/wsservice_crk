#include "stdafx.h"
#include "HexDump.h"

CString HexDump(const BYTE *pData,size_t szData)
{
	CString s;
	for(size_t i=0;i<szData;)
	{
		int j;
		for(j=0;j<16 && (i+j)<szData;j++)
			s.AppendFormat(_T("%02X "),pData[i+j]);
		for(;j<16;j++)
			s+=_T("   ");
		s += _T(" ");
		for(j=0;j<16 && i<szData;j++,i++)
			s += pData[i]>=32 && pData[i]<=0x7F ? (char)pData[i] : '.';
		if (i<szData) s+=_T("\n");
	}
	return s;
}
CStringA StringDumpA(const BYTE *pData,size_t szData)
{
	CStringA s;
	
	PSTR p = s.GetBuffer((int)szData+1);
	if (!p)	AtlThrow(E_OUTOFMEMORY);
	for(size_t i=0;i<szData;i++)
		p[i] = pData[i]>=32 ? (char)pData[i] : '.';
	p[szData]=0;
	s.ReleaseBuffer();
	return s;
}

