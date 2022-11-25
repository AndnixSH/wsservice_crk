#pragma once

#include <winsvc.h>

typedef struct
{
 DWORD dwServiceType;
 DWORD dwStartType;
 DWORD dwDesktop;
 DWORD dwErrorControl;
 LPCTSTR lpBinaryPathName;
 LPCTSTR lpDependencies;
 LPCTSTR lpServiceStartName;
 LPCTSTR lpPassword;
 LPCTSTR lpDisplayName;
} SERV_CONF;
typedef SERV_CONF* LPSERV_CONF;

BOOL RecurseStop(SC_HANDLE SManH, LPCTSTR name);
void FillDefaultServiceParams(LPSERV_CONF cfg);
BOOL ChangeServiceConf(SC_HANDLE ServH, LPSERV_CONF conf);
BOOL ChangeLocalServiceConf(LPCTSTR name, LPSERV_CONF conf);

BOOL start_and_wait_service(SC_HANDLE hSCM,LPCTSTR svc_name,int wait_time_sec);
BOOL start_and_wait_service(LPCTSTR svc_name,int wait_time_sec);
// hServ must be opened with SERVICE_QUERY_CONFIG|SERVICE_QUERY_STATUS|SERVICE_START access
BOOL start_and_wait_service(SC_HANDLE hServ,int wait_time_sec);
