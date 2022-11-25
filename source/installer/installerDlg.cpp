
// installerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "installer.h"
#include "installerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

INT_PTR CALLBACK InstallerDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_INSTALL:
			theApp.DoInstall();
			break;
		case IDC_UNINSTALL:
			theApp.DoUnInstall();
			break;
		case IDCANCEL:
			EndDialog(hDlg, LOWORD(wParam));
			break;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR InstallerDlgShow()
{
	return DialogBox(theApp.m_hInstance,MAKEINTRESOURCE(IDD_INSTALLER_DIALOG),NULL,InstallerDlg);
}
