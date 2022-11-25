#include "stdafx.h"
#include "ServiceHelper.h"


CServiceStatus::CServiceStatus(DWORD dwControlsAccepted,DWORD dwServiceType) throw()
{
	InitializeCriticalSection(&m_ServiceStatusSec);
	m_hServiceStatus=NULL;
	ZeroMemory(&m_ServiceStatus,sizeof(m_ServiceStatus));
	m_ServiceStatus.dwServiceType=dwServiceType;
	m_ServiceStatus.dwControlsAccepted = dwControlsAccepted | SERVICE_ACCEPT_STOP;
	m_ServiceStatus.dwWaitHint=500;
}
CServiceStatus::~CServiceStatus() throw()
{
	LeaveCriticalSection(&m_ServiceStatusSec);
}

void CServiceStatus::SetServiceStatus(DWORD dwState) throw()
{
	EnterCriticalSection(&m_ServiceStatusSec);

	if (m_ServiceStatus.dwCurrentState==dwState)
		m_ServiceStatus.dwCheckPoint++;
	else
	{
		m_ServiceStatus.dwCurrentState=dwState;
		m_ServiceStatus.dwCheckPoint=0;
	}

	::SetServiceStatus(m_hServiceStatus,&m_ServiceStatus);

	LeaveCriticalSection(&m_ServiceStatusSec);
}

void CServiceStatus::ReportCurrentServiceStatus() throw()
{
	EnterCriticalSection(&m_ServiceStatusSec);
	::SetServiceStatus(m_hServiceStatus,&m_ServiceStatus);
	LeaveCriticalSection(&m_ServiceStatusSec);
}

void CServiceStatus::SetServiceWin32Code(DWORD dwExitCode) throw()
{
	EnterCriticalSection(&m_ServiceStatusSec);
	m_ServiceStatus.dwWin32ExitCode=dwExitCode;
	::SetServiceStatus(m_hServiceStatus,&m_ServiceStatus);
	LeaveCriticalSection(&m_ServiceStatusSec);
}



CServiceHandler::CServiceHandler
	(DWORD dwControlsAccepted, DWORD dwServiceType) throw() :
	m_status(dwControlsAccepted,dwServiceType)
{
	
}

void CServiceHandler::Run(LPCTSTR lpServiceName) throw()
{
	SERVICE_STATUS_HANDLE hServiceStatus =
		::RegisterServiceCtrlHandlerEx(lpServiceName,HandlerEx__,this);
	if (!hServiceStatus) return;
	m_status.Init(hServiceStatus);
	m_status.SetServiceStatus(SERVICE_START_PENDING);
	if (BeforeStart())
	{
		m_status.SetServiceStatus(SERVICE_RUNNING);
		Service();
	}
	m_status.SetServiceStatus(SERVICE_STOPPED);
}


DWORD CServiceHandler::HandlerEx__(
   DWORD dwControl,
   DWORD dwEventType,
   LPVOID lpEventData,
   LPVOID lpContext) throw()
{
	return ((CServiceHandler*)lpContext)->HandlerEx_(dwControl,dwEventType,lpEventData);
}

DWORD CServiceHandler::HandlerEx_(
   DWORD dwControl,
   DWORD dwEventType,
   LPVOID lpEventData) throw()
{
	switch (dwControl)
	{
	case SERVICE_CONTROL_STOP:
		StopService();
		return NO_ERROR;
	case SERVICE_CONTROL_INTERROGATE:
		m_status.ReportCurrentServiceStatus();
		return NO_ERROR;
	default:
		return HandlerEx(dwControl,dwEventType,lpEventData);
	}
}


DWORD CServiceHandler::HandlerEx(
	DWORD dwControl,
	DWORD dwEventType,
	LPVOID lpEventData) throw()
{
	return ERROR_CALL_NOT_IMPLEMENTED;
}
BOOL CServiceHandler::BeforeStart()
{
	return TRUE;
}



CServiceHandlerWithMessageLoop::CServiceHandlerWithMessageLoop
	(DWORD dwControlsAccepted, DWORD dwServiceType) :
	CServiceHandler(dwControlsAccepted, dwServiceType)
{
	m_dwServiceThreadId = 0;
}
void CServiceHandlerWithMessageLoop::Service() throw()
{
	m_dwServiceThreadId = GetCurrentThreadId();

	MSG msg;
	while(GetMessage(&msg,NULL,0,0))
		DispatchMessage(&msg);
}
void CServiceHandlerWithMessageLoop::StopService() throw()
{
	m_status.SetServiceStatus(SERVICE_STOP_PENDING);
	PostThreadMessage(m_dwServiceThreadId,WM_QUIT,0,0);
}


CServiceHandlerWithEventWait::CServiceHandlerWithEventWait
	(DWORD dwControlsAccepted, DWORD dwServiceType) :
	CServiceHandler(dwControlsAccepted, dwServiceType)
{
	m_hTermEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
}
CServiceHandlerWithEventWait::~CServiceHandlerWithEventWait() throw()
{
	CloseHandle(m_hTermEvent);
}
void CServiceHandlerWithEventWait::Service() throw()
{
	WaitForSingleObject(m_hTermEvent,INFINITE);
}
void CServiceHandlerWithEventWait::StopService() throw()
{
	m_status.SetServiceStatus(SERVICE_STOP_PENDING);
	SetEvent(m_hTermEvent);
}
