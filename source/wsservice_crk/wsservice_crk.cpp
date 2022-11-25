// wsservice_crk.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "..\common\ServiceHelper.h"
#include "..\common\ApiHook.h"
#include "..\common\HexDump.h"
#include "..\warbird\PeaceBird.h"
#include "dllmain.h"
#include <DSRole.h>
#include <bcrypt.h>
#include <wincrypt.h>
#include <stdio.h>
#include <new>

using namespace ATL;

using namespace PeaceBird;

#define STATUS_INVALID_SIGNATURE         ((NTSTATUS)0xC000A000L)

#define HOOK_EXPORT(DllName,FuncName,RvaJumpback) \
 if (HMODULE h = GetModuleHandle(DllName)) \
 { \
	m_prva##FuncName = GetPEExport(h,#FuncName); \
	if (!m_prva##FuncName) \
		DbgOut(_T("function ") _T(#FuncName) _T(" not found in ") _T(#DllName)); \
	else \
	{ \
		m_rva_old_##FuncName = *m_prva##FuncName; \
		m_f_old_##FuncName = (t_##FuncName)GetProcAddress(h,#FuncName); \
		HookExport(h,m_prva##FuncName,RvaJumpback,my##FuncName); \
		DbgOut(_T("hooked ") _T(#FuncName)); \
	} \
 } \
 else \
 { \
	DbgOut(_T(#DllName) _T(" not loaded")); \
 }
#define UNHOOK_EXPORT(FuncName) \
 if (m_rva_old_##FuncName) \
 { \
	UnhookExport(m_prva##FuncName,m_rva_old_##FuncName); \
	DbgOut(_T("unhooked ") _T(#FuncName)); \
 }

#define HOOK_IMPORT(TargetHMod,DllName,FuncName,FUNC_POPULATION_CALL) \
{ \
	BOOL bDelayImport,bFuncPopulated; \
	m_pimp##FuncName = GetPEImport(hDll,DllName,#FuncName,&bDelayImport,&bFuncPopulated); \
	if (m_pimp##FuncName && \
	*m_pimp##FuncName != my##FuncName) \
	{ \
		if (!bFuncPopulated) \
		{ \
			##FUNC_POPULATION_CALL \
			m_pimp##FuncName = GetPEImport(hDll,DllName,#FuncName,&bDelayImport,&bFuncPopulated); \
			if (!bFuncPopulated) \
			{ \
				DbgOut(_T("couldn't populate ") _T(#FuncName)); \
				m_pimp##FuncName = NULL; \
			} \
		} \
		if (m_pimp##FuncName) \
		{ \
			m_f_old_##FuncName = (t_##FuncName)*m_pimp##FuncName; \
			WriteWithUnprotectPtr(m_pimp##FuncName,my##FuncName); \
			DbgOut(_T("hooked ") _T(#FuncName)); \
		} \
	} \
}
#define UNHOOK_IMPORT(TargetHMod,FuncName) \
if (m_pimp##FuncName) \
{ \
	if (TargetHMod) \
	{ \
		if (*m_pimp##FuncName == my##FuncName) \
		{ \
			*m_pimp##FuncName = m_f_old_##FuncName; \
			DbgOut(_T("unhooked ") _T(#FuncName)); \
		} \
	} \
	m_pimp##FuncName = NULL; \
}
#define INIT_HOOK(FuncName) \
CWSServiceCrkServiceHandler::t_##FuncName \
	CWSServiceCrkServiceHandler::m_f_old_##FuncName=NULL;

#define CATCH_BASIC \
catch (std::bad_alloc) \
{ \
	DbgOut(_T("bad alloc")); \
} \
catch(...) \
{ \
	DbgOut(_T("EXCEPTION. SOMETHING BAD HAS HAPPENED.")); \
}
#define CATCH_COMMON \
catch (CAtlException &atle) \
{ \
	s.Format(_T("error : %08X"),atle.m_hr); \
	DbgOut(s); \
} \
CATCH_BASIC
#define CATCH_PB \
catch (CPeaceBirdException &pbe) \
{ \
	s.Format(_T("error : %s"),pbe.GetMessage()); \
	DbgOut(s); \
} \
CATCH_COMMON


class CWSServiceCrkServiceHandler : public CServiceHandlerWithEventWait
{
public:
	CWSServiceCrkServiceHandler() throw() :
		CServiceHandlerWithEventWait(0)
	{
	}

private:

	static void DbgOut(LPCTSTR ps)
	{
		try
		{
			CString s;
			s.Format(_T("[WsServiceCrk] %s\n"),ps);
			/*
			FILE *F=fopen("C:\\xxx.txt","at");
			if (F)
			{
			_ftprintf(F,_T("%s\n"),s);
			fclose(F);
			}
			*/
			OutputDebugString(s);
		}
		catch(...)
		{
		}
	}

	BOOL BeforeStart() throw()
	{
		// Starting ...

#ifndef _DEBUG
		// this service can run only on Win8/2012 (Win NT 6.2) and higher
		OSVERSIONINFO ovi;
		ovi.dwOSVersionInfoSize = sizeof(ovi);
		GetVersionEx(&ovi);
		if (ovi.dwMajorVersion<6 || ovi.dwMajorVersion>=6 && ovi.dwMinorVersion<2)
		{
			m_status.SetServiceWin32Code(ERROR_OLD_WIN_VERSION);
			return FALSE;
		}
#endif

		return TRUE;
	}

	BOOL DoIdle(DWORD dwMaxWait)
	{
		DWORD dwWaitResult = WaitForSingleObject(m_hTermEvent,dwMaxWait);
		return dwWaitResult==WAIT_TIMEOUT;
	}

	static LPCTSTR sWSDllName;

	typedef NTSTATUS (WINAPI *t_NtSetSystemInformation)
		(IN DWORD SystemInformationClass,
		IN PVOID SystemInformation,
		IN ULONG SystemInformationLength);
	static t_NtSetSystemInformation m_f_old_NtSetSystemInformation;
	DWORD m_rva_old_NtSetSystemInformation;
	PDWORD m_prvaNtSetSystemInformation;
	static NTSTATUS WINAPI myNtSetSystemInformation
		(IN DWORD SystemInformationClass,
		IN PVOID SystemInformation,
		IN ULONG SystemInformationLength)
	{
		DbgOut(_T("called hooked version of NtSetSystemInformation"));

		CString s;

		BOOL SPPQueryProcessingOK = FALSE;
		CChunkedMessage cmq; // decrypted & parsed query
		CWBCipherParams cpq; // cipher params for encrypting response

		if (SystemInformationClass==134) // encrypted spp query
		{
			try
			{
				const PSPQ pspq = (PSPQ)SystemInformation;
				DbgOut(_T("detected encrypted spp query attempt"));
				if (!SystemInformation)
				{
					DbgOut(_T("SystemInformation=NULL"));
				}
				else if (SystemInformationLength!=sizeof(SPQ))
				{
					s.Format(_T("unrecognized SystemInformationLength %u"),SystemInformationLength);
					DbgOut(s);
				}
				else if (!pspq->pQuery || !pspq->szResultBuf)
				{
					DbgOut(_T("query or response buffer = NULL"));
				}
				else
				{
					const void *pc;
					size_t sz;
					CWBEncryptedBlock encblock;
					CAtlArray<BYTE> QueryData;

					s.Format(_T("** encrypted query **\n%s"),HexDump((PBYTE)pspq->pQuery,pspq->szQuery));
					DbgOut(s);

					PB_CHECKED_CALL(encblock.Parse(pc=pspq->pQuery,sz=pspq->szQuery),_T("parsing encrypted query"));
					PB_CHECKED_CALL(Decrypt(encblock,QueryData),_T("decrypting query"));

					s.Format(_T("** decrypted query **\n%s"),HexDump(QueryData.GetData(),QueryData.GetCount()));
					DbgOut(s);

					PB_CHECKED_CALL(cmq.Parse(pc=QueryData.GetData(),sz=QueryData.GetCount()),_T("parsing query ChunkedMessage"));
					PB_CHECKED_CALL(cpq.FromChunkedMessage(cmq),_T("retrieving response cipher params"));

					SPPQueryProcessingOK = TRUE;
				}
			}
			CATCH_PB;
		}

		NTSTATUS status = m_f_old_NtSetSystemInformation(SystemInformationClass,SystemInformation,SystemInformationLength);

		if (SPPQueryProcessingOK)
		{
			try
			{
				if (status!=0)
				{
					s.Format(_T("SPP query was unsuccessful : %08X"),status);
					DbgOut(s);
				}
				else
				{
					const PSPQ pspq = (PSPQ)SystemInformation;
					const void *pc;
					size_t sz;
					CWBEncryptedBlock encblock;
					CAtlArray<BYTE> ResponseData;

					s.Format(_T("** encrypted response **\n%s"),HexDump((PBYTE)pspq->pResultBuf,pspq->szResultBuf));
					DbgOut(s);

					PB_CHECKED_CALL(encblock.Parse(pc=pspq->pResultBuf,sz=pspq->szResultBuf),_T("parsing encrypted response"));
					PB_CHECKED_CALL(Decrypt(encblock,ResponseData),_T("decrypting response"));

					s.Format(_T("** decrypted response **\n%s"),HexDump(ResponseData.GetData(),ResponseData.GetCount()));
					DbgOut(s);

					CChunkedMessage cmr;
					PB_CHECKED_CALL(cmr.Parse(pc=ResponseData.GetData(),sz=ResponseData.GetCount()),_T("parsing response ChunkedMessage"));

					// query chunk 0 - CommandID
					if (cmq.m_Chunks[0].IsDword() && cmq.m_Chunks[0].GetDword()==0 && cmq.m_Chunks.GetCount()>=5)
					{
						// PolicyQuery command
						CStringW SppValueName = cmq.m_Chunks[4].ToWString();
						s.Format(_T("detected spp value query : %ws"),SppValueName);
						DbgOut(s);
						// respone chunk 3 - value data
						if ((SppValueName==L"WSLicensingService-EnableLOBApps" ||
							SppValueName==L"WSLicensingService-LOBSideloadingActivated" ||
							SppValueName==L"Security-SPP-GenuineLocalStatus") &&
							cmr.m_Chunks[3].GetDword()==0)
						{
							DbgOut(_T("about to tamper with policy value"));
							if (cmr.m_Chunks.GetCount()<4 || !cmr.m_Chunks[3].IsDword())
								DbgOut(_T("unexpected chunk structure in response"));
							else
							{
								// change policy value to 1
								cmr.m_Chunks[3].SetDword(1);

								CAtlArray<BYTE> packed_new_res;
								PB_CHECKED_CALL(cmr.Pack(packed_new_res),_T("packing tampered response"));
								CWBEncryptedBlock encblock2;
								PB_CHECKED_CALL(Encrypt(packed_new_res,encblock2,cpq),_T("encrypting tampered response"));
								CAtlArray<BYTE> EncData;
								PB_CHECKED_CALL(encblock2.Pack(EncData),_T("packing encrypted tampered response"));

								if (EncData.GetCount()>pspq->szResultBuf)
								{
									s.Format(_T("tampered encrypted response size > response buffer size (%u != %u)"),EncData.GetCount(),pspq->szResultBuf);
									DbgOut(s);
								}
								else
								{
									memcpy(pspq->pResultBuf,EncData.GetData(),EncData.GetCount());
									DbgOut(_T("successfully tampered with policy value"));
								}
							}
						}
					}
				}
			}
			CATCH_PB;
		}

		return status;
	}

	typedef DWORD (WINAPI *t_DsRoleGetPrimaryDomainInformation)(
		IN  LPCWSTR lpServer OPTIONAL,
		IN  DSROLE_PRIMARY_DOMAIN_INFO_LEVEL InfoLevel,
		OUT PBYTE *Buffer 
		);

	PVOID *m_pimpDsRoleGetPrimaryDomainInformation;
	static t_DsRoleGetPrimaryDomainInformation m_f_old_DsRoleGetPrimaryDomainInformation;

	static DWORD WINAPI myDsRoleGetPrimaryDomainInformation(
		IN  LPCWSTR lpServer OPTIONAL,
		IN  DSROLE_PRIMARY_DOMAIN_INFO_LEVEL InfoLevel,
		OUT PBYTE *Buffer 
		)
	{
		DbgOut(_T("called hooked version of DsRoleGetPrimaryDomainInformation"));

		DWORD dwRes = m_f_old_DsRoleGetPrimaryDomainInformation(lpServer,InfoLevel,Buffer);
		if (dwRes==ERROR_SUCCESS && lpServer==NULL && InfoLevel==DsRolePrimaryDomainInfoBasic)
		{
			PDSROLE_PRIMARY_DOMAIN_INFO_BASIC pinfo = (PDSROLE_PRIMARY_DOMAIN_INFO_BASIC)*Buffer;
			if (pinfo->MachineRole == DsRole_RoleStandaloneWorkstation ||
				pinfo->MachineRole == DsRole_RoleStandaloneServer)
			{
				DbgOut(_T("faking DsRoleGetPrimaryDomainInformation result"));
				pinfo->MachineRole =
					pinfo->MachineRole==DsRole_RoleStandaloneWorkstation ? DsRole_RoleMemberWorkstation : DsRole_RoleMemberServer;
				if (!pinfo->DomainNameFlat)	pinfo->DomainNameFlat = L"DOMAIN";
				if (!pinfo->DomainNameDns)	pinfo->DomainNameDns = L"domain.com";
			}
		}

		return dwRes;
	}


	typedef NTSTATUS (WINAPI *t_BCryptVerifySignature)(
		_In_      BCRYPT_KEY_HANDLE hKey,
		_In_opt_  VOID *pPaddingInfo,
		_In_      PUCHAR pbHash,
		_In_      ULONG cbHash,
		_In_      PUCHAR pbSignature,
		_In_      ULONG cbSignature,
		_In_      ULONG dwFlags
		);
	PVOID *m_pimpBCryptVerifySignature;
	static t_BCryptVerifySignature m_f_old_BCryptVerifySignature;
	static NTSTATUS WINAPI myBCryptVerifySignature(
		_In_      BCRYPT_KEY_HANDLE hKey,
		_In_opt_  VOID *pPaddingInfo,
		_In_      PUCHAR pbHash,
		_In_      ULONG cbHash,
		_In_      PUCHAR pbSignature,
		_In_      ULONG cbSignature,
		_In_      ULONG dwFlags
		)
	{
		DbgOut(_T("called hooked version of BCryptVerifySignature"));

		NTSTATUS ntstatus = m_f_old_BCryptVerifySignature(hKey,pPaddingInfo,pbHash,cbHash,pbSignature,cbSignature,dwFlags);

		try
		{
			CString s;
			if (ntstatus==STATUS_INVALID_SIGNATURE)
			{
				s.Format(_T("invalid signature detected (%08X). faking success."),ntstatus);
				DbgOut(s);
				ntstatus = 0;
			}
			else if (ntstatus==STATUS_INVALID_PARAMETER)
			{
				s.Format(_T("invalid parameter (%08X). faking success."),ntstatus);
				DbgOut(s);
				ntstatus = 0;
			}
			else
			{
				s.Format(_T("status=%08X"),ntstatus);
				DbgOut(s);
			}
		}
		CATCH_BASIC;

		return ntstatus;
	}


	typedef NTSTATUS (WINAPI *t_BCryptHashData)(
		_Inout_  BCRYPT_HASH_HANDLE hHash,
		_In_     PUCHAR pbInput,
		_In_     ULONG cbInput,
		_In_     ULONG dwFlags
		);
	PVOID *m_pimpBCryptHashData;
	static t_BCryptHashData m_f_old_BCryptHashData;
	static NTSTATUS WINAPI myBCryptHashData(
		_Inout_  BCRYPT_HASH_HANDLE hHash,
		_In_     PUCHAR pbInput,
		_In_     ULONG cbInput,
		_In_     ULONG dwFlags
		)
	{
		DbgOut(_T("called hooked version of BCryptHashData"));

		NTSTATUS ntstatus = m_f_old_BCryptHashData(hHash,pbInput,cbInput,dwFlags);

		CString s;
		try
		{
			s.Format(_T("ntstatus=%08X. Hashed data : \n%S"),ntstatus,StringDumpA(pbInput,cbInput));
			DbgOut(s);
		}
		CATCH_COMMON;

		return ntstatus;
	}



	BOOL HackWS()
	{
		HMODULE hDll = GetModuleHandle(sWSDllName);
		if (!hDll)
			return FALSE; // dll not present

		DbgOut(_T("starting hooking API"));

		#define POPULATE_DsRoleGetPrimaryDomainInformation	((t_DsRoleGetPrimaryDomainInformation)*m_pimpDsRoleGetPrimaryDomainInformation)(0,(DSROLE_PRIMARY_DOMAIN_INFO_LEVEL)-1,0);
		HOOK_IMPORT(hDll,"dsrole.dll",DsRoleGetPrimaryDomainInformation,POPULATE_DsRoleGetPrimaryDomainInformation)
		#define POPULATE_BCryptHashData	((t_BCryptHashData)*m_pimpBCryptHashData)(0,0,0,0);
		HOOK_IMPORT(hDll,"bcrypt.dll",BCryptHashData,POPULATE_BCryptHashData)
		#define POPULATE_BCryptVerifySignature ((t_BCryptVerifySignature)*m_pimpBCryptVerifySignature)(0,0,0,0,0,0,0);
		HOOK_IMPORT(hDll,"bcrypt.dll",BCryptVerifySignature,POPULATE_BCryptVerifySignature)

		return
			m_pimpDsRoleGetPrimaryDomainInformation &&
			m_pimpBCryptHashData &&
			m_pimpBCryptVerifySignature;
	}
	void UnHackWS()
	{
		HMODULE hDll = GetModuleHandle(sWSDllName);
		UNHOOK_IMPORT(hDll,DsRoleGetPrimaryDomainInformation)
		UNHOOK_IMPORT(hDll,BCryptVerifySignature)
		UNHOOK_IMPORT(hDll,BCryptHashData)
	}

	void HackExports()
	{
		HOOK_EXPORT(_T("ntdll.dll"),NtSetSystemInformation,0xFF0)
	}
	void UnhackExports()
	{
		UNHOOK_EXPORT(NtSetSystemInformation)
	}

	void Service() throw()
	{
		// Started

#ifdef _DEBUG
		//Sleep(20000); // give some time to attach debugger
		//LoadLibraryA("wsservice.dll");
#endif

		// we dont want to miss DLL appearance
		// and want to hook api as soon as possible
		// it is needed because some early checks can be important
		SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_ABOVE_NORMAL);

		HackExports();

		DbgOut(_T("Starting watch loop"));
		BOOL bWSDllPresentNow, bWSDllPresent = FALSE;
		for(;;)
		{
			bWSDllPresentNow = GetModuleHandle(sWSDllName) != NULL;
			if (!bWSDllPresent && bWSDllPresentNow)
			{
				DbgOut(_T("wsservice.dll appeared"));
				HackWS();
			}
			else if (bWSDllPresent && !bWSDllPresentNow)
			{
				DbgOut(_T("wsservice.dll gone"));
			}
			bWSDllPresent = bWSDllPresentNow;

			if (!DoIdle(bWSDllPresent ? 500 : 1)) break;
		}

		// stopping ...

		DbgOut(_T("Watch loop stopped"));
		UnHackWS();
		UnhackExports();
	}
};

LPCTSTR CWSServiceCrkServiceHandler::sWSDllName = _T("wsservice.dll");
INIT_HOOK(DsRoleGetPrimaryDomainInformation)
INIT_HOOK(BCryptVerifySignature)
INIT_HOOK(BCryptHashData)
INIT_HOOK(NtSetSystemInformation)

extern "C" VOID WINAPI ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv)
{
	CWSServiceCrkServiceHandler handler;
	handler.Run(lpszArgv[0]);
}
