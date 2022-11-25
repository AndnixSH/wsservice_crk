#pragma once

#include "ApiHook.h"

// Write to possibly VirtualProtect-ed location (code or readonly section)
void WriteWithUnprotect(LPVOID lpDst,LPCVOID lpSrc,SIZE_T size);
void WriteWithUnprotectDword(LPVOID lpDst,DWORD dw);
void WriteWithUnprotectPtr(LPVOID lpDst,LPCVOID Ptr);

typedef struct
{
	DWORD Attributs;
	DWORD Name;
	DWORD ModuleHandle;
	DWORD ImportAddressTable;
	DWORD ImportNameTable;
	DWORD BoundDelayImportTable;
	DWORD UnloadDelayImportTable;
	DWORD TimeStamp;
} IMAGE_DELAY_IMPORT_DESCRIPTOR,*PIMAGE_DELAY_IMPORT_DESCRIPTOR;

// hMod = module where to search import table
// pbDelayImport - TRUE if function is found in delay import
// pbFuncPopulated - TRUE if function import is bound to real dll (delay imports can point to DLL loading procedure)
// on return - pointer to location where function address is stored in PE import table. it can be modified to intercept function.
// if not found - GetLastError()=ERROR_PROC_NOT_FOUND
PVOID *GetPEImport(HMODULE hMod,const char *DllName,const char *FuncName,PBOOL pbDelayImport=NULL,PBOOL pbFuncPopulated=NULL);

// return pointer to RVA of exported function
PDWORD GetPEExport(HMODULE hMod,const char *FuncName);

#if	defined(_M_X64) || defined(_M_IX86)
// to hook export function some free space inside target dll is required
// use end of first page of hMod. its usually free there.
// payload size : x86 - 5 bytes, x64 - 14 bytes
// pExportRva - address returned by GetPEExport()
// RvaForJumpback - where to put jumpback code
void HookExport(HMODULE hMod,PDWORD pExportRva,DWORD RvaForJumpback,const void *pHookFunc);
void UnhookExport(PDWORD pExportRva,DWORD RvaOriginalFunc);
#endif
