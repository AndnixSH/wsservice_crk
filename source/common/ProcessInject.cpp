#include "stdafx.h"
#include "ProcessInject.h"
#include <psapi.h>

HMODULE FindDllInProcess(HANDLE hProcess,LPCTSTR lpDllName)
{
	HMODULE hm[256];
	DWORD dwcbNeeded,dw;
	WCHAR DllName[MAX_PATH];

	if (!EnumProcessModules(hProcess,hm,sizeof(hm),&dwcbNeeded))
		return NULL;
	for (int i=0;i<_countof(hm);i++)
	{
		if (dw = GetModuleBaseName(hProcess,hm[i],DllName,_countof(DllName)))
		{
			if (!_tcsicmp(DllName,lpDllName))
				return hm[i];
		}
	}
	return NULL;
};

struct INJDATA
{
	WCHAR DllName[MAX_PATH];
};

BOOL InjectDll(DWORD dwPid,LPWSTR swInjectDll)
{
	HANDLE hProcess;

	hProcess = ::OpenProcess(	PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | 
								PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ,
								FALSE, dwPid);
	if (hProcess == NULL)
		return FALSE;

	INJDATA *pDataRemote = NULL;
	HANDLE hThread = NULL;
	BOOL bRes = FALSE;

	__try {
		// 1. Allocate memory in the remote process for the injected INJDATA & NewProc 
		//	  (note that they are placed right one after the other)
		// 2. Write a copy of NewProc and the initialized INJDATA to the allocated memory
		size_t injsize = sizeof(INJDATA);

		pDataRemote = (INJDATA*) VirtualAllocEx( hProcess, 0, injsize, MEM_COMMIT, PAGE_EXECUTE_READWRITE );		
		if (!pDataRemote)
			__leave;

		SIZE_T szWr;

		INJDATA injdata;
		ZeroMemory(&injdata,sizeof(injdata));
		wcscpy_s(injdata.DllName,_countof(injdata.DllName),swInjectDll);

		if (!WriteProcessMemory( hProcess, pDataRemote, &injdata, sizeof(injdata), &szWr))
			__leave;
		
		
		HINSTANCE hKernel32 = GetModuleHandle(_T("kernel32.dll"));
		SIZE_T rvaLoadLibraryW = (SIZE_T)GetProcAddress(hKernel32,"LoadLibraryW") - (SIZE_T)hKernel32;

		HINSTANCE hKernel32Remote = FindDllInProcess(hProcess,_T("kernel32.dll"));
		if (!hKernel32Remote)
			__leave;

		PVOID pRemoteLoadLibraryW = (PBYTE)hKernel32Remote + rvaLoadLibraryW;

		DWORD dwThreadId;
			
		// Start execution of remote InjectFunc
		hThread = CreateRemoteThread(hProcess, NULL, 0, 
				(LPTHREAD_START_ROUTINE) pRemoteLoadLibraryW,
				pDataRemote->DllName, 0 , &dwThreadId);

		if ( !hThread )
			__leave;

		Sleep(500);

		bRes = TRUE;
	}
	__finally {
		if( pDataRemote )
			VirtualFreeEx( hProcess, pDataRemote, 0, MEM_RELEASE );
		if ( hThread != 0 )			
			CloseHandle(hThread);
	}
	CloseHandle( hProcess );
	return bRes;
}

