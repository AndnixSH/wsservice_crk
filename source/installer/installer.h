
// installer.h : main header file for the PROJECT_NAME application
//

#pragma once

#include "resource.h"		// main symbols

void AfxMessageBox(
  LPCTSTR lpText,
  UINT uType);

class CCustomCommandLineInfo
{
public:
  CCustomCommandLineInfo();

  BOOL m_bInst,m_bUnInst;
protected:
  void Parse();
  LPWSTR *m_argv;
  int m_argc;
};


class CinstallerApp
{
public:
	CinstallerApp();

// Overrides
public:
	virtual int Run();

	HINSTANCE m_hInstance;
protected:
	int m_exitcode;
	CAtlString m_SourcePath,m_WSDllTargetWithFullPath;
	CCustomCommandLineInfo m_cli;

public:
	BOOL DoInstall();
	BOOL DoUnInstall();
};

extern CinstallerApp theApp;
