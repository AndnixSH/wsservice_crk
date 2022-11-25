// wsll.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <locale.h>
#include "..\common\utf8.h"

extern "C" {
HRESULT WINAPI WSLicenseOpen(PVOID *pContext);
HRESULT WINAPI WSLicenseClose(PVOID pContext);
HRESULT WINAPI WSEvaluatePackage(PVOID pContext,int a,SIZE_T b);
HRESULT WINAPI WSLicenseInstallLicense(PVOID pContext,DWORD cbSize,LPCSTR pLicenseXml,PVOID pUnk);
HRESULT WINAPI WSLicenseUninstallLicense(PVOID pContext,PCWSTR pPtr1,const GUID *pGuid);
HRESULT WINAPI WSLicenseRefreshLicense(PVOID pContext,const VOID *pUnk);
HRESULT WINAPI WSLicenseGetLicensesForProducts(PVOID pContext, DWORD cbGuidCount, const GUID *pGuidArray, HLOCAL *phMem);
HRESULT WINAPI WSLicenseGetAllValidAppCategoryIds(PVOID pContext, PDWORD pdwCount , HLOCAL *phMem);
}

CString errstrfromcode(LPCTSTR sWhat,DWORD dwErr)
{
	CString s;
	LPTSTR lpMsgBuf;
	if (FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		dwErr,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), /* Default language */
		(LPTSTR) &lpMsgBuf,
		0,
		NULL
		))
	{
		if (sWhat)
			s.Format(_T("%s : %s"),sWhat,lpMsgBuf);
		else
			s=lpMsgBuf;
		LocalFree( lpMsgBuf );
	}
	else
	{
		if (sWhat) 
			s.Format(_T("%s : "),sWhat);
		if (dwErr>=0x40000000)
			s.AppendFormat(_T("error %08X"),dwErr);
		else
			s.AppendFormat(_T("error %u"),dwErr);
	}
	return s;
}

void printerr(LPCTSTR sWhat,DWORD dwErr)
{
	_tprintf(_T("%s\n"),errstrfromcode(sWhat,dwErr));
}
void printlasterr(LPCTSTR sWhat)
{
	printerr(sWhat,GetLastError());
}

HRESULT UninstallLicense(PVOID pContext,const GUID *pGuid)
{
	HRESULT hRes;
	switch (hRes = WSLicenseUninstallLicense(pContext,NULL,pGuid))
	{
	case S_OK :
		_tprintf(_T("OK\n"));
		break;
	case 0x80070490 :
		_tprintf(_T("license not found\n"));
		break;
	default:
		_tprintf(_T("%s\n"),errstrfromcode(NULL,hRes));
	}
	return hRes;
}
HRESULT UninstallLicense(const GUID *pGuid)
{
	HRESULT hRes;
	PVOID pContext;
	
	hRes = WSLicenseOpen(&pContext);
	if (FAILED(hRes))
	{
		_tprintf(_T("%s\n"),errstrfromcode(_T("WSLicenseOpen"),hRes));
		return hRes;
	}
	_tprintf(_T("Uninstalling license : "));
	hRes = UninstallLicense(pContext,pGuid);
	WSLicenseClose(pContext);
	return hRes;

}

HRESULT InstallLicense(PVOID pContext,LPCSTR sLic,BOOL bTest=FALSE)
{
	CStringA sbuf(sLic);

	HRESULT hRes;
	CLSID clsid;

	ZeroMemory(&clsid,sizeof(clsid));

	// try to find ProductID and first uninstall current license
	int pos2,pos1 = sbuf.Find("<ProductID>");
	if (pos1>=0)
	{
		pos1+=11;
		pos2 = sbuf.Find("</ProductID>");
		if (pos2>=0 && pos2>pos1)
		{
			CStringW pid;
			pid = "{" + sbuf.Mid(pos1,pos2-pos1) + "}";
			hRes = CLSIDFromString(pid,&clsid);
			if (SUCCEEDED(hRes))
			{
				_tprintf(_T("Uninstalling license for ProductID %s : "),pid);
				hRes=UninstallLicense(pContext,&clsid);
			}
		}
	}

	_tprintf(_T("Installing license : "));

	hRes = WSLicenseInstallLicense(pContext,sbuf.GetLength(),(LPCSTR)sbuf,NULL);
	if (FAILED(hRes))
	{
		_tprintf(_T("%s\n"),errstrfromcode(NULL,hRes));
		return hRes;
	}
	_tprintf(_T("OK\n"),hRes);

	if (bTest)
	{
		_tprintf(_T("Uninstalling test license : "));
		hRes=UninstallLicense(pContext,&clsid);
	}

	return hRes;
}
HRESULT InstallLicense(LPCSTR sLic,BOOL bTest=FALSE)
{
	HRESULT hRes;
	PVOID pContext;
	
	hRes = WSLicenseOpen(&pContext);
	if (FAILED(hRes))
	{
		_tprintf(_T("%s\n"),errstrfromcode(_T("WSLicenseOpen"),hRes));
		return hRes;
	}
	hRes = InstallLicense(pContext,sLic,bTest);
	WSLicenseClose(pContext);
	return hRes;

}





HRESULT EnumProducts(PVOID pContext,CAtlArray<GUID> &ProductList)
{
	HRESULT hRes;
	HLOCAL hMem;
	DWORD dwCount;

	hRes = WSLicenseGetAllValidAppCategoryIds(pContext, &dwCount,&hMem);
	if (FAILED(hRes))
		return hRes;
	ProductList.SetCount(dwCount);
	memcpy(ProductList.GetData(),hMem,sizeof(GUID)*dwCount);
	LocalFree(hMem);
	return hRes;
}

HRESULT GetLicenseForProduct(PVOID pContext,const GUID *pGuid,CStringW &sLicense)
{
	HRESULT hRes;
	HLOCAL hMem;

	hRes=WSLicenseGetLicensesForProducts(pContext,1,pGuid, &hMem);
	if (FAILED(hRes)) return hRes;

	LPCSTR psLic = ((PCSTR*)hMem)[0];
	if (!psLic)
		return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);

	if (!ConvertUTF8ToUTF16(psLic,sLicense))
		return HRESULT_FROM_WIN32(GetLastError());

	LocalFree(((HLOCAL*)hMem)[0]);

	return hRes;
}



HRESULT PrintLicenses(PVOID pContext)
{
	HRESULT hRes;
	CAtlArray<GUID> ProductList;
	CStringW sLicense;
	WCHAR wsguid[39];

	if (FAILED(hRes=EnumProducts(pContext,ProductList)))
	{
		_tprintf(_T("%s\n"),errstrfromcode(_T("EnumProducts"),hRes));
	}
	else
	{
		for(size_t i=0;i<ProductList.GetCount();i++)
		{
			*wsguid=0;
			StringFromGUID2(ProductList[i],wsguid,_countof(wsguid));
			wprintf(L"*** License for ProductID=%s\n\n",wsguid);
			hRes = GetLicenseForProduct(pContext,&ProductList[i],sLicense);
			if (FAILED(hRes))
			{
				_tprintf(_T("%s\n\n"),errstrfromcode(_T("GetLicenseForProduct"),hRes));
			}
			else
			{
				wprintf(L"%s\n\n",sLicense);
			}
		}
	}
	return hRes;
}
HRESULT PrintLicenses()
{
	HRESULT hRes;
	PVOID pContext;
	
	hRes = WSLicenseOpen(&pContext);
	if (FAILED(hRes))
	{
		_tprintf(_T("%s\n"),errstrfromcode(_T("WSLicenseOpen"),hRes));
		return hRes;
	}

	hRes = PrintLicenses(pContext);

	WSLicenseClose(pContext);
	return hRes;

}




static LPCSTR TestLic = "<License Version=\"1\" Source=\"OEM\" xmlns=\"urn:schemas-microsoft-com:windows:store:licensing:ls\"><Binding Binding_Type=\"Machine\"><ProductID>aaaaaaaa-1111-2222-3333-444444444444</ProductID><PFM>TestPackage_aaaaaaaaaaaaa</PFM></Binding><LicenseInfo Type=\"Full\"><IssuedDate>2022-11-01T16:29:14Z</IssuedDate><LastUpdateDate>2022-11-01T16:29:14Z</LastUpdateDate></LicenseInfo><Signature xmlns=\"http://www.w3.org/2000/09/xmldsig#\"><SignedInfo><CanonicalizationMethod Algorithm=\"http://www.w3.org/2001/10/xml-exc-c14n#\" /><SignatureMethod Algorithm=\"http://www.w3.org/2001/04/xmldsig-more#rsa-sha256\" /><Reference URI=\"\"><Transforms><Transform Algorithm=\"http://www.w3.org/2000/09/xmldsig#enveloped-signature\" /></Transforms><DigestMethod Algorithm=\"http://www.w3.org/2001/04/xmlenc#sha256\" /><DigestValue>UT0YLYT7N0zx8hOrC1+eO9sUDXPNRniF64DH9ppiMGw=</DigestValue></Reference></SignedInfo><SignatureValue>Z4jSCZKb0fMAaH/kf0h2CTuU+QWBv9fhAX1aV21oujgx0QkF0YDaXykR3zlrH/hQU20qFUWTJw69rvBVptTnd4yC/KoKIjobWZK1oojPjBjbzLQkwWfART7uzXsKXXIsLzKk8mNg/6o0WKHBET8hq/JSLAIkT3hxcYIQtGmsCQWJuEBWovjNeuu1TcgIXrXkVpUzsGU6yURqtI/wIPiUEFP+f8XkmxU23cfDdJfOg8kCiQhi3a0JRyu8vn96zuCZXhsGVg8lZd7ighJNklsK6impUdAFJEKxU3vwJLk8wRcvUgcypS0y9E02hZJM4CvTPwLSzx6dWmGmKDcwEEqO1Q==</SignatureValue></Signature></License>";

int _tmain(int argc, _TCHAR* argv[])
{
	HRESULT hRes;

	setlocale( LC_ALL, "" );

	_tprintf(_T("Windows Store License Loader v1.3.0\n\n"));

	if (argc<2)
	{
		_tprintf(_T("wsll [-n] <license_file>  (load license. -n = skip test bogus license)\nwsll -l  (list licenses)\nwsll -u <ProductID>  (uninstall license)\n"));
		return 1;
	}

	// list licenses
	if (!_tcsicmp(argv[1],_T("-l")))
	{
		hRes = PrintLicenses();
		return FAILED(hRes) ? 3 : 0;
	}

	// uninstall license
	if (!_tcsicmp(argv[1],_T("-u")))
	{
		if (argc<3)
		{
			_tprintf(_T("ProductID required\n"));
			return 1;
		}

		CStringW spid = CString(_T("{")) + argv[2] + _T("}");
		GUID gpid;

		if (FAILED(hRes=CLSIDFromString(spid,&gpid)))
		{
			_tprintf(_T("Invalid format of ProductID. Must specify GUID without {}.\n"));
			return 1;
		}
		hRes = UninstallLicense(&gpid);
		return FAILED(hRes) ? 3 : 0;
	}

	// load license
	BOOL bTestLicense;
	LPCTSTR lpLicFileName;
	if (!_tcsicmp(argv[1],_T("-n")))
	{
		if (argc<3)
		{
			_tprintf(_T("License file name required\n"));
			return 1;
		}
		bTestLicense = FALSE;
		lpLicFileName = argv[2];
	}
	else
	{
		bTestLicense = TRUE;
		lpLicFileName = argv[1];
	}

	HANDLE hFile=CreateFile(lpLicFileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
	if (hFile==INVALID_HANDLE_VALUE)
	{
		printlasterr(_T("CreateFile"));
		return 2;
	}

	DWORD dwSize = SetFilePointer(hFile,0,NULL,FILE_END);
	SetFilePointer(hFile,0,NULL,FILE_BEGIN);

	CStringA sbuf;

	DWORD rd=0;
	LPSTR pStrBuf = sbuf.GetBuffer(dwSize);
	ReadFile(hFile,pStrBuf,dwSize,&rd,NULL);
	pStrBuf[rd]=0;
	sbuf.ReleaseBuffer();

	CloseHandle(hFile);

	if (bTestLicense)
	{
		_tprintf(_T("* Trying test bogus license first\n"));
		hRes = InstallLicense(TestLic,TRUE);
		if (FAILED(hRes))
		{
			_tprintf(_T("Test bogus license failed. Did you install WSServiceCrk ?\n"),hRes);
		}
		if (FAILED(hRes))
			return 3;
	}
	_tprintf(_T("* Installing license from the file\n"));
	hRes = InstallLicense(sbuf,FALSE);

	return FAILED(hRes) ? 3 : 0;
}

