// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <dsrole.h>
#include <bcrypt.h>
#include <dbghelp.h>
#include <wincrypt.h>
#include "..\common\ApiHook.h"

#include <atlstr.h>


void print_imp_dir(HMODULE hMod,SIZE_T rvaNameList,SIZE_T rvaAdrList)
{
	PCHAR pMod = (PCHAR)hMod;

	PIMAGE_IMPORT_BY_NAME pIBN;
	for(SIZE_T *IBN = (PSIZE_T) (pMod + rvaNameList),	*Adr = (PSIZE_T) (pMod + rvaAdrList), i=0;
		IBN[i];
		i++)
	{
		pIBN = PIMAGE_IMPORT_BY_NAME(pMod+IBN[i]);
		printf("[%04X] %s : 0x%I64x\n",pIBN->Hint,pIBN->Name ? pIBN->Name : "(none)",Adr[i]);
	}
}

typedef NTSTATUS (WINAPI *t_NtSetSystemInformation)
 (IN DWORD SystemInformationClass,
  IN PVOID SystemInformation,
  IN ULONG SystemInformationLength);
t_NtSetSystemInformation f_old_NtSetSystemInformation;
DWORD rva_old_NtSetSystemInformation;
NTSTATUS WINAPI myNtSetSystemInformation
 (IN DWORD SystemInformationClass,
  IN PVOID SystemInformation,
  IN ULONG SystemInformationLength)
{
	NTSTATUS status = f_old_NtSetSystemInformation(SystemInformationClass,SystemInformation,SystemInformationLength);
	return status;
}


typedef DWORD (WINAPI *t_DsRoleGetPrimaryDomainInformation)(
    IN  LPCWSTR lpServer OPTIONAL,
    IN  DSROLE_PRIMARY_DOMAIN_INFO_LEVEL InfoLevel,
    OUT PBYTE *Buffer 
    );
PVOID *pimpDsRoleGetPrimaryDomainInformation;
t_DsRoleGetPrimaryDomainInformation f_old_DsRoleGetPrimaryDomainInformation;
DWORD WINAPI myDsRoleGetPrimaryDomainInformation(
    IN  LPCWSTR lpServer OPTIONAL,
    IN  DSROLE_PRIMARY_DOMAIN_INFO_LEVEL InfoLevel,
    OUT PBYTE *Buffer 
    )
{
	OutputDebugString(_T("Called hooked version of DsRoleGetPrimaryDomainInformation"));
	printf("!!! In func hook\n");

	DWORD dwRes = f_old_DsRoleGetPrimaryDomainInformation(lpServer,InfoLevel,Buffer);
	if (dwRes==ERROR_SUCCESS && lpServer==NULL && InfoLevel==DsRolePrimaryDomainInfoBasic)
	{
		PDSROLE_PRIMARY_DOMAIN_INFO_BASIC pinfo = (PDSROLE_PRIMARY_DOMAIN_INFO_BASIC)*Buffer;
		if (pinfo->MachineRole == DsRole_RoleStandaloneWorkstation ||
			pinfo->MachineRole == DsRole_RoleStandaloneServer)
		{
			pinfo->MachineRole =
				pinfo->MachineRole==DsRole_RoleStandaloneWorkstation ? DsRole_RoleMemberWorkstation : DsRole_RoleMemberServer;
			if (!pinfo->DomainNameFlat)	pinfo->DomainNameFlat = L"DOMAIN";
			if (!pinfo->DomainNameDns)	pinfo->DomainNameDns = L"domain.com";
		}
	}

	return dwRes;
}

typedef BOOL (WINAPI *t_CertVerifyCertificateChainPolicy)(
	LPCSTR pszPolicyOID,
	PCCERT_CHAIN_CONTEXT pChainContext,
	PCERT_CHAIN_POLICY_PARA pPolicyPara,
	PCERT_CHAIN_POLICY_STATUS pPolicyStatus);

PVOID *pimpCertVerifyCertificateChainPolicy;
t_CertVerifyCertificateChainPolicy f_old_CertVerifyCertificateChainPolicy;

BOOL WINAPI myCertVerifyCertificateChainPolicy(
	LPCSTR pszPolicyOID,
	PCCERT_CHAIN_CONTEXT pChainContext,
	PCERT_CHAIN_POLICY_PARA pPolicyPara,
	PCERT_CHAIN_POLICY_STATUS pPolicyStatus)
{
	FILE *F=fopen("C:\\CertVerifyCertificateChainPolicy.txt","at");
	if (F)
	{
		fprintf(F,"pszPolicyOID=%I64X pChainContext=%I64X pPolicyPara=%I64X pPolicyStatus=%I64X\n",
         pszPolicyOID,pChainContext,pPolicyPara,pPolicyStatus);
		fclose(F);
	}
	if (pPolicyStatus && pPolicyStatus->cbSize>=sizeof(CERT_CHAIN_POLICY_STATUS))
	{
		pPolicyStatus->dwError = 0;
		pPolicyStatus->lChainIndex = -1;
		pPolicyStatus->lElementIndex = -1;
		pPolicyStatus->pvExtraPolicyStatus = NULL;
	}
	return TRUE; // skip verify
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
PVOID *pimpBCryptVerifySignature;
t_BCryptVerifySignature f_old_BCryptVerifySignature;
NTSTATUS WINAPI myBCryptVerifySignature(
  _In_      BCRYPT_KEY_HANDLE hKey,
  _In_opt_  VOID *pPaddingInfo,
  _In_      PUCHAR pbHash,
  _In_      ULONG cbHash,
  _In_      PUCHAR pbSignature,
  _In_      ULONG cbSignature,
  _In_      ULONG dwFlags
)
{
	FILE *F=fopen("C:\\bcryptHashData.txt","at");
	if (F)
	{
		fprintf(F,"hKey=%I64X pPaddingInfo=%I64X pbHash=%I64X cbHash=%u pbSignature=%I64X cbSignature=%u dwFlags=%08X\n",
         hKey,pPaddingInfo,pbHash,cbHash,pbSignature,cbSignature,dwFlags);
		fclose(F);
	}
	return 0;
}

typedef NTSTATUS (WINAPI *t_BCryptHashData)(
  _Inout_  BCRYPT_HASH_HANDLE hHash,
  _In_     PUCHAR pbInput,
  _In_     ULONG cbInput,
  _In_     ULONG dwFlags
);
PVOID *pimpBCryptHashData;
t_BCryptHashData f_old_BCryptHashData;
NTSTATUS WINAPI myBCryptHashData(
  _Inout_  BCRYPT_HASH_HANDLE hHash,
  _In_     PUCHAR pbInput,
  _In_     ULONG cbInput,
  _In_     ULONG dwFlags
)
{
	NTSTATUS ntstatus = f_old_BCryptHashData(hHash,pbInput,cbInput,dwFlags);
	FILE *F=fopen("C:\\bcrypthashdata.txt","at");
	if (F)
	{
		fprintf(F,"ntstatus=%08X hHash=%I64X pbInput=%I64X cbInput=%u dwFlags=%08X\n",
         ntstatus,hHash,pbInput,cbInput,dwFlags);
		if (pbInput)
		{
			fprintf(F,"*pbInput = ");
			for(ULONG u=0;u<cbInput;u++)
				fprintf(F,"%02X ",pbInput[u]);
			fprintf(F,"\n");
		}
		fclose(F);
	}
	return 0;
}


int _tmain(int argc, _TCHAR* argv[])
{
		HMODULE hDll = GetModuleHandle(NULL);
		PCHAR pDll = (PCHAR)hDll;

		ULONG uSz;
		BOOL b;
		NTSTATUS ntstatus;

		BOOL bDelayImport,bFuncPopulated;

		///////////

		pimpDsRoleGetPrimaryDomainInformation =
			GetPEImport(hDll,"dsrole.dll","DsRoleGetPrimaryDomainInformation",&bDelayImport,&bFuncPopulated);
		if (!pimpDsRoleGetPrimaryDomainInformation)
		{
			printf("Function not found\n");
			return 10;
		}
		if (!bFuncPopulated)
		{
			// call it with errorneus parameters just to trigger delay loading
			((t_DsRoleGetPrimaryDomainInformation)*pimpDsRoleGetPrimaryDomainInformation)(0,(DSROLE_PRIMARY_DOMAIN_INFO_LEVEL)-1,0);
			pimpDsRoleGetPrimaryDomainInformation =
				GetPEImport(hDll,"dsrole.dll","DsRoleGetPrimaryDomainInformation",&bDelayImport,&bFuncPopulated);
			if (!bFuncPopulated)
			{
				printf("Couldn't populate DLL\n");
				return 11;
			}
		}
		// remember old addr
		f_old_DsRoleGetPrimaryDomainInformation = (t_DsRoleGetPrimaryDomainInformation)*pimpDsRoleGetPrimaryDomainInformation;

		*pimpDsRoleGetPrimaryDomainInformation = myDsRoleGetPrimaryDomainInformation; // hook
		PDSROLE_PRIMARY_DOMAIN_INFO_BASIC pdi;
		if (!DsRoleGetPrimaryDomainInformation(NULL, DsRolePrimaryDomainInfoBasic, (PBYTE*)&pdi))
		{
			 printf("(hooked) DomainDNS=%ws\n",pdi->DomainNameDns ? pdi->DomainNameDns : L"<NULL>");
			 DsRoleFreeMemory(pdi);
		}
		*pimpDsRoleGetPrimaryDomainInformation = f_old_DsRoleGetPrimaryDomainInformation; //unhook
		if (!DsRoleGetPrimaryDomainInformation(NULL, DsRolePrimaryDomainInfoBasic, (PBYTE*)&pdi))
		{
			 printf("(not hooked) DomainDNS=%ws\n",pdi->DomainNameDns ? pdi->DomainNameDns : L"<NULL>");
			 DsRoleFreeMemory(pdi);
		}

		///////////

		CERT_CHAIN_POLICY_STATUS ccps;
		memset(&ccps,0,sizeof(ccps));

		pimpCertVerifyCertificateChainPolicy =
			GetPEImport(hDll,"crypt32.dll","CertVerifyCertificateChainPolicy",&bDelayImport,&bFuncPopulated);
		if (!pimpCertVerifyCertificateChainPolicy)
		{
			printf("Function not found\n");
			return 10;
		}
		if (!bFuncPopulated)
		{
			// call it with errorneus parameters just to trigger delay loading
			((t_CertVerifyCertificateChainPolicy)*pimpCertVerifyCertificateChainPolicy)(0,0,0,&ccps);
			pimpCertVerifyCertificateChainPolicy =
				GetPEImport(hDll,"crypt32.dll","CertVerifyCertificateChainPolicy",&bDelayImport,&bFuncPopulated);
			if (!bFuncPopulated)
			{
				printf("Couldn't populate DLL\n");
				return 11;
			}
		}
		// remember old addr
		f_old_CertVerifyCertificateChainPolicy = (t_CertVerifyCertificateChainPolicy)*pimpCertVerifyCertificateChainPolicy;

		*pimpCertVerifyCertificateChainPolicy = myCertVerifyCertificateChainPolicy; // hook
		b=CertVerifyCertificateChainPolicy(0,0,0,&ccps);
		*pimpCertVerifyCertificateChainPolicy = f_old_CertVerifyCertificateChainPolicy; //unhook
		b=CertVerifyCertificateChainPolicy(0,0,0,&ccps);

		///////////

		pimpBCryptVerifySignature =
			GetPEImport(hDll,"bcrypt.dll","BCryptVerifySignature",&bDelayImport,&bFuncPopulated);
		if (!pimpBCryptVerifySignature)
		{
			printf("Function not found\n");
			return 10;
		}
		if (!bFuncPopulated)
		{
			// call it with errorneus parameters just to trigger delay loading
			((t_BCryptVerifySignature)*pimpBCryptVerifySignature)(0,0,0,0,0,0,0);
			pimpBCryptVerifySignature =
				GetPEImport(hDll,"crypt32.dll","BCryptVerifySignature",&bDelayImport,&bFuncPopulated);
			if (!bFuncPopulated)
			{
				printf("Couldn't populate DLL\n");
				return 11;
			}
		}
		// remember old addr
		f_old_BCryptVerifySignature = (t_BCryptVerifySignature)*pimpBCryptVerifySignature;

		*pimpBCryptVerifySignature = myBCryptVerifySignature; // hook
		ntstatus = BCryptVerifySignature(0,0,0,0,0,0,0);
		*pimpBCryptVerifySignature = f_old_BCryptVerifySignature; //unhook
		ntstatus = BCryptVerifySignature(0,0,0,0,0,0,0);

		///////////


		pimpBCryptHashData =
			GetPEImport(hDll,"bcrypt.dll","BCryptHashData",&bDelayImport,&bFuncPopulated);
		if (!pimpBCryptHashData)
		{
			printf("Function not found\n");
			return 10;
		}
		if (!bFuncPopulated)
		{
			// call it with errorneus parameters just to trigger delay loading
			((t_BCryptHashData)*pimpBCryptHashData)(0,0,0,0);
			pimpBCryptHashData =
				GetPEImport(hDll,"crypt32.dll","BCryptHashData",&bDelayImport,&bFuncPopulated);
			if (!bFuncPopulated)
			{
				printf("Couldn't populate DLL\n");
				return 11;
			}
		}
		// remember old addr
		f_old_BCryptHashData = (t_BCryptHashData)*pimpBCryptHashData;

		*pimpBCryptHashData = myBCryptHashData; // hook
		ntstatus = BCryptHashData(0,0,0,0);
		*pimpBCryptHashData = f_old_BCryptHashData; //unhook
		ntstatus = BCryptHashData(0,0,0,0);

		///////////

		/*
		hDll = GetModuleHandleA("ntdll.dll");
		void *pf1 = GetProcAddress(hDll,"A_SHAFinal");
		PIMAGE_EXPORT_DIRECTORY pexp = (PIMAGE_EXPORT_DIRECTORY)ImageDirectoryEntryToData(hDll,TRUE,IMAGE_DIRECTORY_ENTRY_EXPORT,&uSz);
		*/

		/*

		for(int i=0;i<15;i++)
		{
			PBYTE p = (PBYTE)ImageDirectoryEntryToData(hDll,TRUE,i,&uSz);
			if (p)
			{
			SIZE_T s = p-(PBYTE)hDll;
			printf("%i - %08X\n",i,s);
			s=0;
			}
		}

		*/


		PIMAGE_IMPORT_DESCRIPTOR p = (PIMAGE_IMPORT_DESCRIPTOR)ImageDirectoryEntryToData(hDll,TRUE,IMAGE_DIRECTORY_ENTRY_IMPORT,&uSz);
		while (p->Name)
		{
			printf("IMP from %s\n",pDll+p->Name);
			print_imp_dir(hDll,p->OriginalFirstThunk,p->FirstThunk);
			p++;
		}

		/*

		{
		PIMAGE_DELAY_IMPORT_DESCRIPTOR pd = (PIMAGE_DELAY_IMPORT_DESCRIPTOR)ImageDirectoryEntryToData(hDll,TRUE,IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT,&uSz);
		while (pd->Name)
		{
			printf("delay IMP from %s\n",pDll+pd->Name);
			print_imp_dir(hDll,pd->ImportNameTable,pd->ImportAddressTable);
			pd++;
		}
		}
		*/

	PDWORD ppf;
	HMODULE hNtdll = GetModuleHandle(_T("ntdll.dll"));
	f_old_NtSetSystemInformation = (t_NtSetSystemInformation)GetProcAddress(hNtdll,"NtSetSystemInformation");
	ppf = GetPEExport(hNtdll,"NtSetSystemInformation");
	rva_old_NtSetSystemInformation = *ppf;
	HookExport(hNtdll,ppf,0xFF0,myNtSetSystemInformation);
	t_NtSetSystemInformation f_NtSetSystemInformation = (t_NtSetSystemInformation)GetProcAddress(hNtdll,"NtSetSystemInformation");
	NTSTATUS sss = f_NtSetSystemInformation(1000,NULL,0);
	UnhookExport(ppf,rva_old_NtSetSystemInformation);
	f_NtSetSystemInformation = (t_NtSetSystemInformation)GetProcAddress(hNtdll,"NtSetSystemInformation");
	sss = f_NtSetSystemInformation(1000,NULL,0);


	return 0;
}

