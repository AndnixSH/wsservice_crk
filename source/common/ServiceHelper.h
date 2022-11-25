#pragma once

class CServiceStatus
{
private:
	CRITICAL_SECTION m_ServiceStatusSec;
	SERVICE_STATUS m_ServiceStatus;
	SERVICE_STATUS_HANDLE m_hServiceStatus;
public:
	CServiceStatus
		(DWORD dwControlsAccepted, DWORD dwServiceType=SERVICE_WIN32_OWN_PROCESS) throw();
	~CServiceStatus() throw();

	inline void Init(SERVICE_STATUS_HANDLE hServiceStatus) throw()
	{
		m_hServiceStatus=hServiceStatus;
	}

	void SetServiceStatus(DWORD dwState) throw();
	void ReportCurrentServiceStatus() throw();
	void SetServiceWin32Code(DWORD dwExitCode) throw();
	inline DWORD GetExitCode() throw() {return m_ServiceStatus.dwWin32ExitCode;}
};


class CServiceHandler
{
	friend class CServiceHandler;
private:
	static DWORD WINAPI HandlerEx__(
		DWORD dwControl,
		DWORD dwEventType,
		LPVOID lpEventData,
		LPVOID lpContext) throw();
protected:
	CServiceStatus m_status;

	DWORD HandlerEx_(
		DWORD dwControl,
		DWORD dwEventType,
		LPVOID lpEventData) throw();
	virtual DWORD HandlerEx(
		DWORD dwControl,
		DWORD dwEventType,
		LPVOID lpEventData) throw();

	// TRUE - if success, FALSE if failed
	virtual BOOL BeforeStart() throw(); // (START_PENDING)
	// must not return until service want to stop
	virtual void Service()  throw() = 0; // (RUNNING)
	// must force Service() to exit
	virtual void StopService() throw() = 0;

public:

	CServiceHandler
		(DWORD dwControlsAccepted, DWORD dwServiceType=SERVICE_WIN32_OWN_PROCESS) throw();
	void Run(LPCTSTR lpServiceName) throw();
	inline DWORD GetExitCode() throw() {return m_status.GetExitCode();}
};

// if message processing is required
class CServiceHandlerWithMessageLoop : public CServiceHandler
{
public:
	CServiceHandlerWithMessageLoop
		(DWORD dwControlsAccepted, DWORD dwServiceType=SERVICE_WIN32_OWN_PROCESS) throw();
private:
	DWORD m_dwServiceThreadId;

	void Service() throw();
	void StopService() throw();
};

// if message processing is not required required
class CServiceHandlerWithEventWait : public CServiceHandler
{
public:
	CServiceHandlerWithEventWait
		(DWORD dwControlsAccepted, DWORD dwServiceType=SERVICE_WIN32_OWN_PROCESS) throw();
	~CServiceHandlerWithEventWait() throw();
protected:
	HANDLE m_hTermEvent;

	void Service() throw();
	void StopService() throw();
};

