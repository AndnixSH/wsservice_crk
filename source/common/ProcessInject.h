#pragma once

HMODULE FindDllInProcess(HANDLE hProcess,LPCTSTR lpDllName);
BOOL InjectDll(DWORD dwPid,LPWSTR swInjectDll);
