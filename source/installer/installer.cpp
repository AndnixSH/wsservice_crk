
// installer.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "installer.h"
#include "installerDlg.h"

#include "..\common\Wow64.h"
#include "..\common\serv.h"
#include "..\common\VersionInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void AfxMessageBox(
  LPCTSTR lpText,
  UINT uType)
{
	MessageBox(GetActiveWindow(),lpText,_T("Installer"),uType);
}


static LPCTSTR WSCrkServiceDllName = _T("wsservice_crk.dll");
static LPCTSTR WSCrkServiceName = _T("WSServiceCrk");
static LPCTSTR WSCrkServiceDisplayName = _T("Windows Store service crack");
static LPCTSTR WSCrkServiceSvchostGroup = _T("LocalServiceAndNoImpersonation");
static LPCTSTR WSCrkServiceMain = _T("ServiceMain");

static LPCTSTR SvchostKey = _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\svchost");


void ErrMBox(LPCTSTR sWhat,DWORD dwErr)
{
	CAtlString s;
	if (dwErr)
	{
		LPVOID lpMsgBuf;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			dwErr,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), /* Default language */
			(LPTSTR) &lpMsgBuf,
			0,
			NULL
			);
		s.Format(_T("%s : %s"),sWhat,lpMsgBuf);
		LocalFree( lpMsgBuf );
	}
	else
		s=sWhat;

	AfxMessageBox(s,MB_OK|MB_ICONERROR);
}

BOOL SearchMultiSZ(LPCTSTR lpData,LPCTSTR lpString,LPCTSTR *pstrFound=NULL)
{
	BOOL bWSSvcFound=FALSE;
	while (*lpData)
	{
		if (!_tcsicmp(lpData,lpString))
		{
			if (pstrFound) *pstrFound = lpData;
			return TRUE;
		}
		lpData+=_tcslen(lpData)+1;
	}
	return FALSE;
}


CCustomCommandLineInfo::CCustomCommandLineInfo()
{
	m_bInst = FALSE;
	m_bUnInst = FALSE;

	m_argv = CommandLineToArgvW(GetCommandLine(),&m_argc);

	Parse();
}
void CCustomCommandLineInfo::Parse()
{
	if (m_argc>=2)
	{
		if(!_wcsicmp(m_argv[1], L"inst"))
			m_bInst = TRUE;
		else if(!_wcsicmp(m_argv[1], L"uninst"))
			m_bUnInst = TRUE;
	}
};

// CinstallerApp construction

CinstallerApp::CinstallerApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance

	m_exitcode = 0;
}


// The one and only CinstallerApp object

CinstallerApp theApp;


// CinstallerApp initialization

BOOL CinstallerApp::Run()
{
	LPTSTR p = m_SourcePath.GetBuffer(MAX_PATH);
	GetModuleFileName(NULL,p,MAX_PATH);
	PathRemoveFileSpec(p);
	m_SourcePath.ReleaseBuffer();

	GetSystemDirectory(m_WSDllTargetWithFullPath.GetBuffer(MAX_PATH),MAX_PATH);
	m_WSDllTargetWithFullPath.ReleaseBuffer();
	m_WSDllTargetWithFullPath.AppendFormat(_T("\\%s"),WSCrkServiceDllName);


	if (m_cli.m_bInst)
	{
		// unattended install
		DoInstall();
		return m_exitcode;
	}
	if (m_cli.m_bUnInst)
	{
		// unattended uninstall
		DoUnInstall();
		return m_exitcode;
	}

	InstallerDlgShow();

	return m_exitcode;
}


BOOL CinstallerApp::DoInstall()
{
	DWORD dwErr;
	HKEY hk;
	LONG lRes;
	CAtlString s;
	SC_HANDLE SManH = NULL;
	SC_HANDLE ServH = NULL;

	for(;;)
	{
		// allow sideloading in group policy

		LPCTSTR PolicyKey=_T("SOFTWARE\\Policies\\Microsoft\\Windows\\Appx");
		if (ERROR_SUCCESS!=(lRes=RegCreateKeyEx(HKEY_LOCAL_MACHINE,
			PolicyKey,
			0,0,0,KEY_SET_VALUE,NULL,&hk,NULL)))
		{
			SetLastError(lRes);
			s.Format(_T("CreateKey %s"),PolicyKey);
			break;
		}

		DWORD dw=1;
		RegSetValueEx(hk,_T("AllowAllTrustedApps"),0,REG_DWORD,(PBYTE)&dw,sizeof(DWORD));

		RegCloseKey(hk);

		// for safety disable some critical scheduled tasks
		WinExec("schtasks /change /disable /TN \"\\Microsoft\\Windows\\WS\\License Validation\"",0);
		WinExec("schtasks /change /disable /TN \"\\Microsoft\\Windows\\WS\\WSRefreshBannedAppsListTask\"",0);

		//
		CAtlString sWSDllWithFullPath = m_SourcePath+_T("\\")+WSCrkServiceDllName;

		// check version of wsservicecrk.dll. do not overwrite newer version.
		CFileVersionInfo vi_src,vi_tgt;
		if (vi_tgt.LoadFromFile(m_WSDllTargetWithFullPath))
		{
			DWORD dwVerTgt,dwVerSrc;
			if (!vi_src.LoadFromFile(sWSDllWithFullPath))
			{
				s.Format(_T("LoadVersionInfo for %s"),sWSDllWithFullPath);
				break;
			}
			if (!vi_src.GetFileVersion(&dwVerSrc))
			{
				s.Format(_T("GetFileVersion for %s"),sWSDllWithFullPath);
				break;
			}
			if (!vi_tgt.GetFileVersion(&dwVerTgt))
			{
				s.Format(_T("GetFileVersion for %s"),m_WSDllTargetWithFullPath);
				break;
			}
			if (dwVerTgt>dwVerSrc)
			{
				s=_T("You have newer version installed. Installation aborted.");
				SetLastError(ERROR_SUCCESS);
				break;
			}
		}


		// install WSServiceCrk
		SManH = OpenSCManager(NULL,NULL,SC_MANAGER_CREATE_SERVICE);

		if (!SManH)
		{
			s=_T("OpenSCManager");
			break;
		}
		// stop service if it exists
		if (RecurseStop(SManH,WSCrkServiceName))
		{
			Sleep(500); // just in case : svchost can report stopped state but dll still mapped
			dwErr = ERROR_SUCCESS;
		}
		else
		{
			dwErr = GetLastError();
			if (dwErr!=ERROR_SERVICE_DOES_NOT_EXIST && dwErr!=ERROR_SERVICE_NOT_ACTIVE)
			{
				s.Format(_T("Attempt to stop service %s"),WSCrkServiceName);
				break;
			}
		}
		// install dll
		if (!CopyFile(sWSDllWithFullPath,m_WSDllTargetWithFullPath,FALSE))
		{
			s.Format(_T("Copy %s to %s"),WSCrkServiceDllName,m_WSDllTargetWithFullPath);
			break;
		}
		if (dwErr==ERROR_SERVICE_DOES_NOT_EXIST)
		{
			// update svchost key

			if (ERROR_SUCCESS!=(lRes=RegOpenKeyEx(HKEY_LOCAL_MACHINE,SvchostKey,0,KEY_QUERY_VALUE|KEY_SET_VALUE,&hk)))
			{
				SetLastError(lRes);
				s.Format(_T("OpenKey %s"),SvchostKey);
				break;
			}
			BYTE DataBuf[512+128];
			DWORD dwSize = 512;
			if (ERROR_SUCCESS!=(lRes=RegQueryValueEx(hk,WSCrkServiceSvchostGroup,0,NULL,DataBuf,&dwSize)))
			{
				RegCloseKey(hk);
				SetLastError(lRes);
				s.Format(_T("QueryValue %s\\%s"),SvchostKey,WSCrkServiceSvchostGroup);
				break;
			}

			if (!SearchMultiSZ((LPCTSTR)DataBuf,WSCrkServiceName))
			{
				// append WSCrkServiceName to REG_MULTI_SZ list
				LPTSTR lpEnd = (LPTSTR)(DataBuf+dwSize)-1;
				DWORD l = (DWORD)_tcslen(WSCrkServiceName);
				memcpy(lpEnd,WSCrkServiceName,(l+1)*sizeof(TCHAR));
				lpEnd[l+1]=0;
				dwSize += (l+1)*sizeof(TCHAR);
				RegSetValueEx(hk,WSCrkServiceSvchostGroup,0,REG_MULTI_SZ,DataBuf,dwSize);
			}
			RegCloseKey(hk);

			// create service

			CAtlString sBinaryPath;
			sBinaryPath.Format(_T("%%SystemRoot%%\\System32\\svchost.exe -k %s"),WSCrkServiceSvchostGroup);

			ServH = CreateService(
				SManH,WSCrkServiceName,WSCrkServiceDisplayName,SERVICE_ALL_ACCESS,
				SERVICE_WIN32_SHARE_PROCESS,SERVICE_AUTO_START,SERVICE_ERROR_NORMAL,
				sBinaryPath,NULL,NULL,NULL,_T("NT AUTHORITY\\LocalService"),NULL);
			if (!ServH)
			{
				s.Format(_T("Create service %s"),WSCrkServiceName);
				break;
			}

			SERVICE_REQUIRED_PRIVILEGES_INFO srpi;
			srpi.pmszRequiredPrivileges = _T("SeCreateGlobalPrivilege\0SeChangeNotifyPrivilege\0");
			if (!ChangeServiceConfig2(ServH,SERVICE_CONFIG_REQUIRED_PRIVILEGES_INFO,&srpi))
			{
				s.Format(_T("Change service config for %s"),WSCrkServiceName);
				break;
			}

			CloseServiceHandle(ServH); ServH = NULL;

			// stop all the services in the group to free svchost
			CAtlList<CAtlString> ServicesForRestart;
			LPTSTR lpData=(LPTSTR)DataBuf;
			while (*lpData)
			{
				if (RecurseStop(SManH,lpData))
					ServicesForRestart.AddHead(lpData);
				lpData+=_tcslen(lpData)+1;
			}
			// restart previously running services
			POSITION pos = ServicesForRestart.GetHeadPosition();
			while(pos)
			{
				const CAtlString &ServName = ServicesForRestart.GetNext(pos);
				if (ServH = OpenService(SManH,ServName,SERVICE_START))
				{
					StartService(ServH,0,NULL);
					CloseServiceHandle(ServH); ServH=NULL;
				}
			}

		}

		// update service parameters
		{
			CAtlString sWSSvcParams;
			sWSSvcParams.Format(_T("SYSTEM\\CurrentControlSet\\Services\\%s\\Parameters"),WSCrkServiceName);
			if (ERROR_SUCCESS!=(lRes=RegCreateKeyEx(HKEY_LOCAL_MACHINE,
				sWSSvcParams,
				0,0,0,KEY_SET_VALUE,NULL,&hk,NULL)))
			{
				SetLastError(lRes);
				s.Format(_T("CreateKey %s"),sWSSvcParams);
				break;
			}

			RegSetValueEx(hk,_T("ServiceDll"),0,REG_EXPAND_SZ,(PBYTE)(LPCTSTR)m_WSDllTargetWithFullPath,(m_WSDllTargetWithFullPath.GetLength()+1)*sizeof(TCHAR));
			RegSetValueEx(hk,_T("ServiceMain"),0,REG_SZ,(PBYTE)WSCrkServiceMain,((DWORD)_tcslen(WSCrkServiceMain)+1)*sizeof(TCHAR));
			DWORD dw=1;
			RegSetValueEx(hk,_T("ServiceDllUnloadOnStop"),0,REG_DWORD,(PBYTE)&dw,sizeof(DWORD));

			RegCloseKey(hk);
		}


		if (!start_and_wait_service(SManH,WSCrkServiceName,15))
		{
			s.Format(_T("Start service %s"),WSCrkServiceName);
			break;
		}

		CloseServiceHandle(SManH);

		AfxMessageBox(_T("WSService crack was successfully installed !"),MB_OK|MB_ICONINFORMATION);

		m_exitcode = 0;

		return TRUE;
	}

	m_exitcode = GetLastError();

	ErrMBox(s,GetLastError());
	if (ServH) CloseServiceHandle(ServH);
	if (SManH) CloseServiceHandle(SManH);

	return FALSE;
}

BOOL CinstallerApp::DoUnInstall()
{
	DWORD dwErr;
	HKEY hk;
	LONG lRes;
	CAtlString s;
	SC_HANDLE SManH = OpenSCManager(NULL,NULL,0);
	SC_HANDLE ServH = NULL;

	for(;;)
	{
		if (!SManH)
		{
			s=_T("OpenSCManager");
			break;;
		}
		// stop service if it exists
		if (RecurseStop(SManH,WSCrkServiceName))
		{
			Sleep(500); // just in case : svchost can report stopped state but dll still mapped
			dwErr = ERROR_SUCCESS;
		}
		else
		{
			dwErr = GetLastError();
			if (dwErr!=ERROR_SERVICE_DOES_NOT_EXIST && dwErr!=ERROR_SERVICE_NOT_ACTIVE)
			{
				s.Format(_T("Attempt to stop service %s"),WSCrkServiceName);
				break;
			}
		}
		// remove dll
		if (!DeleteFile(m_WSDllTargetWithFullPath))
		{
			if (GetLastError()!=ERROR_FILE_NOT_FOUND)
			{
				s.Format(_T("DeleteFile %s"),m_WSDllTargetWithFullPath);
				break;
			}
		}
		if (dwErr!=ERROR_SERVICE_DOES_NOT_EXIST)
		{
			// update svchost key

			if (ERROR_SUCCESS!=(lRes=RegOpenKeyEx(HKEY_LOCAL_MACHINE,SvchostKey,0,KEY_QUERY_VALUE|KEY_SET_VALUE,&hk)))
			{
				SetLastError(lRes);
				s.Format(_T("OpenKey %s"),SvchostKey);
				break;
			}
			BYTE DataBuf[512+128];
			DWORD dwSize = 512;
			if (ERROR_SUCCESS!=(lRes=RegQueryValueEx(hk,WSCrkServiceSvchostGroup,0,NULL,DataBuf,&dwSize)))
			{
				RegCloseKey(hk);
				SetLastError(lRes);
				s.Format(_T("QueryValue %s\\%s"),SvchostKey,WSCrkServiceSvchostGroup);
				break;
			}

			LPCTSTR lpstrFound;
			if (SearchMultiSZ((LPCTSTR)DataBuf,WSCrkServiceName,&lpstrFound))
			{
				DWORD l = (DWORD)_tcslen(lpstrFound)+1;
				memcpy(const_cast<LPTSTR>(lpstrFound),lpstrFound+l,dwSize - ((PBYTE)lpstrFound-DataBuf) - l*sizeof(TCHAR));
				dwSize -= l*sizeof(TCHAR);
				RegSetValueEx(hk,WSCrkServiceSvchostGroup,0,REG_MULTI_SZ,DataBuf,dwSize);
			}
			RegCloseKey(hk);

			// delete service

			ServH = OpenService(SManH,WSCrkServiceName,DELETE);
			if (!ServH)
			{
				s.Format(_T("Open service %s"),WSCrkServiceName);
				break;
			}
			if (!DeleteService(ServH))
			{
				s.Format(_T("Delete service %s"),WSCrkServiceName);
				break;
			}
			CloseServiceHandle(ServH); ServH = NULL;
		}

		CloseServiceHandle(SManH);

		AfxMessageBox(_T("WSServiceCrk was successfully removed !"),MB_OK|MB_ICONINFORMATION);

		m_exitcode = 0;

		return TRUE;
	}

	m_exitcode = GetLastError();

	ErrMBox(s,GetLastError());
	if (ServH) CloseServiceHandle(ServH);
	if (SManH) CloseServiceHandle(SManH);

	return FALSE;
}


int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	if (IsWow64())
	{
		AfxMessageBox(_T("You must run x64 version of installer"),MB_ICONERROR);
		return 1000;
	}
	theApp.m_hInstance = hInstance;
	return theApp.Run();
}


