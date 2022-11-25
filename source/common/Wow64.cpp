#include "stdafx.h"
#include "wow64.h"


typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

BOOL IsWow64()
{
    BOOL bIsWow64 = FALSE;

	LPFN_ISWOW64PROCESS 
		fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(
			GetModuleHandle(_T("kernel32")),"IsWow64Process");
    if (fnIsWow64Process)
		fnIsWow64Process(GetCurrentProcess(),&bIsWow64);
    return bIsWow64;
}
BOOL IsSystemX64()
{
#ifdef _M_X64
	return TRUE;
#else
	return IsWow64();
#endif
}
