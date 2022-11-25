#include "stdafx.h"
#include "ApiHook.h"

#include <DbgHelp.h>
#include <Psapi.h>

void WriteWithUnprotect(LPVOID lpDst,LPCVOID lpSrc,SIZE_T size)
{
	__try
	{
		memcpy(lpDst,lpSrc,size);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		DWORD dwOldProtect;
		if (!VirtualProtect(lpDst,size,PAGE_READWRITE,&dwOldProtect))
			throw;
		memcpy(lpDst,lpSrc,size);
		VirtualProtect(lpDst,size,dwOldProtect,&dwOldProtect);
	}

}
void WriteWithUnprotectPtr(LPVOID lpDst,LPCVOID Ptr)
{
	return WriteWithUnprotect(lpDst,&Ptr,sizeof(Ptr));
}
void WriteWithUnprotectDword(LPVOID lpDst,DWORD dw)
{
	return WriteWithUnprotect(lpDst,&dw,sizeof(DWORD));
}


static PVOID *SearchImports(HMODULE hMod,SIZE_T rvaNameList,SIZE_T rvaAdrList,const char *FuncName)
{
	PCHAR pMod = (PCHAR)hMod;

	PIMAGE_IMPORT_BY_NAME pIBN;
	for(SIZE_T *IBN = (PSIZE_T) (pMod + rvaNameList),	*Adr = (PSIZE_T) (pMod + rvaAdrList), i=0;
		IBN[i];
		i++)
	{
		pIBN = PIMAGE_IMPORT_BY_NAME(pMod+IBN[i]);
		if (pIBN->Name && ! strcmp(FuncName,pIBN->Name))
			return (PVOID*)(Adr+i);
	}
	return NULL;
}
PVOID *GetPEImport(HMODULE hMod,const char *DllName,const char *FuncName,PBOOL pbDelayImport,PBOOL pbFuncPopulated)
{
	PCHAR pMod = (PCHAR)hMod;
	ULONG uSz;

	PIMAGE_IMPORT_DESCRIPTOR pi = (PIMAGE_IMPORT_DESCRIPTOR)ImageDirectoryEntryToData(hMod,TRUE,IMAGE_DIRECTORY_ENTRY_IMPORT,&uSz);
	if (pi)
	{
		for (;pi->Name;pi++)
		{
			if (!_stricmp(DllName,pMod+pi->Name))
			{
				PVOID *FuncAddr=SearchImports(hMod,pi->OriginalFirstThunk,pi->FirstThunk,FuncName);
				if (pbDelayImport) *pbDelayImport=FALSE;
				if (pbFuncPopulated) *pbFuncPopulated=TRUE;
				return FuncAddr;
			}
		}
	}

	PIMAGE_DELAY_IMPORT_DESCRIPTOR pdi = (PIMAGE_DELAY_IMPORT_DESCRIPTOR)ImageDirectoryEntryToData(hMod,TRUE,IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT,&uSz);
	if (pdi)
	{
		for (;pdi->Name;pdi++)
		{
			if (!_stricmp(DllName,pMod+pdi->Name))
			{
				PVOID *FuncAddr=SearchImports(hMod,pdi->ImportNameTable,pdi->ImportAddressTable,FuncName);
				if (FuncAddr)
				{
					if (pbDelayImport) *pbDelayImport=TRUE;
					if (pbFuncPopulated)
					{
						// if is populated it must point to target dll (it also must be mapped)
						HMODULE hTargetMod = GetModuleHandleA(DllName);
						if (!hTargetMod)
							*pbFuncPopulated = FALSE;
						else
						{
							MODULEINFO modInfo;
							if (!GetModuleInformation(GetCurrentProcess(),hTargetMod,&modInfo,sizeof(modInfo)))
								return NULL;
							*pbFuncPopulated = (SIZE_T)*FuncAddr>(SIZE_T)hTargetMod && (SIZE_T)*FuncAddr<((SIZE_T)hTargetMod+modInfo.SizeOfImage);
						}
					}
				}
				return FuncAddr;
			}
		}
	}

	SetLastError(ERROR_PROC_NOT_FOUND);
	return NULL;
};


// function from ReactOS source
static WORD WINAPI NameToOrdinal
	(HMODULE hDll,
	 LPCSTR ImportName,
     DWORD NumberOfNames,
     const DWORD *NameTable,
     const WORD *OrdinalTable)
{
    LONG Start, End, Next, CmpResult;

    /* Use classical binary search to find the ordinal */
    Start = Next = 0;
    End = NumberOfNames - 1;
    while (End >= Start)
    {
        /* Next will be exactly between Start and End */
        Next = (Start + End) >> 1;

        /* Compare this name with the one we need to find */
        CmpResult = strcmp(ImportName, (PCHAR)((PBYTE)hDll + NameTable[Next]));

        /* We found our entry if result is 0 */
        if (!CmpResult) break;

        /* We didn't find, update our range then */
        if (CmpResult < 0)
            End = Next - 1;
        else if (CmpResult > 0)
            Start = Next + 1;
    }

    /* If end is before start, then the search failed */
    if (End < Start) return -1;

    /* Return found name */
    return OrdinalTable[Next];
}

PDWORD GetPEExport(HMODULE hMod,const char *FuncName)
{
	PCHAR pMod = (PCHAR)hMod;
	DWORD ExpDirSize;
	PIMAGE_EXPORT_DIRECTORY pexp = (PIMAGE_EXPORT_DIRECTORY)ImageDirectoryEntryToData(hMod,TRUE,IMAGE_DIRECTORY_ENTRY_EXPORT,&ExpDirSize);
	if (pexp)
	{
		WORD Ordinal = NameToOrdinal(hMod,FuncName,pexp->NumberOfNames,(PDWORD)(pMod+pexp->AddressOfNames),(PWORD)(pMod+pexp->AddressOfNameOrdinals));
		if (Ordinal==-1) return NULL; // function was not found
		PDWORD FuncAddrs = (PDWORD)(pMod+pexp->AddressOfFunctions);
		return FuncAddrs + Ordinal;
	}
	return NULL;
}

#if	defined(_M_X64) || defined(_M_IX86)
// sorry for not having this for ARM - I have no winARM tablet for testing
// but its easy - add here ARM-specific jumpback code
void HookExport(HMODULE hMod,PDWORD pExportRva,DWORD RvaForJumpback,const void *pHookFunc)
{
	PVOID pJumpbackPayload = (PBYTE)hMod+RvaForJumpback;

#ifdef _M_X64
	BYTE payload[14];
	// jmp dword/qword ptr [$]
	payload[0] = 0xFF;
	payload[1] = 0x25;
	payload[2] = 0x0;
	payload[3] = 0x0;
	payload[4] = 0x0;
	payload[5] = 0x0;
	*(const void**)(payload+6) = pHookFunc;
#else
	BYTE payload[5];
	// jmp im32
	payload[0] = 0xE9;
	*(size_t*)(payload+1) = (PBYTE)pHookFunc-(PBYTE)pJumpbackPayload-5;
#endif

	WriteWithUnprotect(pJumpbackPayload,payload,sizeof(payload));

	MEMORY_BASIC_INFORMATION mbi;
	if (VirtualQuery(pJumpbackPayload,&mbi,sizeof(mbi)))
	{
		// enable execute if not enabled
		if (!(mbi.Protect & 0xF0))
			VirtualProtect(pJumpbackPayload,sizeof(payload),mbi.Protect & 0xFFFFFF00 | (mbi.Protect & 0xF)<<4,&mbi.Protect);
	}

	WriteWithUnprotectDword(pExportRva,RvaForJumpback);
}
void UnhookExport(PDWORD pExportRva,DWORD RvaOriginalFunc)
{
	WriteWithUnprotectDword(pExportRva,RvaOriginalFunc);
}

#endif
