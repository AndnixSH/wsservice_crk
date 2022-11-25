#include "stdafx.h"
#include "serv.h"

#define CheckInterval 500
#define CheckTimes    120

static BOOL RecurseStop_(SC_HANDLE SManH, LPCTSTR name, int depth)
{
BOOL pend;
SERVICE_STATUS ServStat;
LPENUM_SERVICE_STATUS Enum;
DWORD i,BytesNeeded,EntriesReturned;
SC_HANDLE ServH;

ServH=OpenService(SManH,name,SERVICE_STOP | SERVICE_QUERY_STATUS |
				  SERVICE_ENUMERATE_DEPENDENTS);
if (!ServH) return FALSE;
try
{
if (!EnumDependentServices(ServH,SERVICE_ACTIVE,NULL,0,&BytesNeeded,&EntriesReturned))
  {
   if (GetLastError()!=ERROR_MORE_DATA) throw FALSE;
   Enum=(LPENUM_SERVICE_STATUS)LocalAlloc(LMEM_FIXED,BytesNeeded);
   if (!EnumDependentServices(ServH,SERVICE_ACTIVE,
	    Enum,BytesNeeded,&BytesNeeded,&EntriesReturned)) 
     {LocalFree(Enum); throw FALSE;}
   for (i=0;i<EntriesReturned;i++) 
	 if (!RecurseStop_(SManH,Enum[i].lpServiceName,depth+1)) {LocalFree (Enum); throw FALSE;}
   LocalFree(Enum);
  }
if (!ControlService(ServH,SERVICE_CONTROL_STOP,&ServStat)) 
  {
	if (GetLastError()!=ERROR_SERVICE_NOT_ACTIVE) throw FALSE;
	throw (int)(depth>0);
  }
//printf ("stopping %s .",name);
pend=FALSE;
for (i=0;i<CheckTimes;i++)
   {
	 QueryServiceStatus(ServH,&ServStat);
	 if (ServStat.dwCurrentState==SERVICE_STOPPED) break;
	 if ((!pend) && (ServStat.dwCurrentState==SERVICE_STOP_PENDING))
	   {
		 //printf (" pending ");
		 pend=TRUE;
	   }
	 if (ServStat.dwCurrentState==SERVICE_RUNNING) break;
	 Sleep(CheckInterval);
	 //printf(".");
   }
if (ServStat.dwCurrentState!=SERVICE_STOPPED)
  {
   //printf (" service did not stop !\n");
   //if (ServStat.dwCurrentState!=SERVICE_STOP_PENDING) PrintServError(&ServStat);
   throw FALSE;
  }
 //else
  //printf (" done\n");
}
catch(int b)
{
 DWORD err = GetLastError();
 CloseServiceHandle(ServH);
 SetLastError(err);
 return b;
}
CloseServiceHandle(ServH);
return TRUE;
}

BOOL RecurseStop(SC_HANDLE SManH, LPCTSTR name)
{
 return RecurseStop_(SManH,name,0);
}

void FillDefaultServiceParams(LPSERV_CONF cfg)
{
/* prefill structure with default values */
cfg->dwServiceType=SERVICE_WIN32_OWN_PROCESS;
cfg->dwStartType=SERVICE_DEMAND_START;
cfg->dwDesktop=SERVICE_NO_CHANGE;
cfg->dwErrorControl=SERVICE_ERROR_NORMAL;
cfg->lpBinaryPathName=NULL;
cfg->lpDependencies=NULL;
cfg->lpServiceStartName=NULL;
cfg->lpPassword=NULL;
cfg->lpDisplayName=NULL;
}

BOOL ChangeServiceConf(SC_HANDLE ServH, LPSERV_CONF conf)
{
DWORD servtype;
if (conf->dwServiceType==SERVICE_NO_CHANGE)
  {
	if(conf->dwDesktop==SERVICE_NO_CHANGE)
	  servtype=SERVICE_NO_CHANGE;
	 else
	  {
       LPQUERY_SERVICE_CONFIG ServConf;
       DWORD BytesNeeded;
       QueryServiceConfig(ServH,NULL,0,&BytesNeeded);
       ServConf=(LPQUERY_SERVICE_CONFIG)LocalAlloc(LMEM_FIXED,BytesNeeded);
       if (!QueryServiceConfig(ServH,ServConf,BytesNeeded,&BytesNeeded))
         {LocalFree(ServConf); return FALSE;};
       if (conf->dwDesktop)
         servtype = ServConf->dwServiceType | SERVICE_INTERACTIVE_PROCESS;
        else
	     servtype = ServConf->dwServiceType & (~SERVICE_INTERACTIVE_PROCESS);
       LocalFree(ServConf);
	  }
  }
 else
  if(conf->dwDesktop==SERVICE_NO_CHANGE)
	 servtype = conf->dwServiceType;
   else
     servtype = conf->dwServiceType | conf->dwDesktop;
return ChangeServiceConfig(ServH,
					       servtype,
					       conf->dwStartType,
					       conf->dwErrorControl,
					       conf->lpBinaryPathName,
					       NULL, /* lpLoadOrderGroup */
					       NULL, /* lpdwTagId */
					       conf->lpDependencies,
					       conf->lpServiceStartName,
					       conf->lpPassword,
					       conf->lpDisplayName);
}

BOOL ChangeLocalServiceConf(LPCTSTR name, LPSERV_CONF conf)
{
SC_HANDLE SManH,ServH;
BOOL br;

if (!(SManH=OpenSCManager (NULL,NULL,SC_MANAGER_CREATE_SERVICE))) return FALSE;
if (!(ServH=OpenService(SManH,name,SERVICE_QUERY_CONFIG | SERVICE_CHANGE_CONFIG))) 
 {CloseServiceHandle(SManH); return FALSE;}
DWORD dwErr = GetLastError();
br=ChangeServiceConf(ServH,conf);
CloseServiceHandle(ServH); CloseServiceHandle(SManH);
SetLastError(dwErr);
return br;
}


BOOL start_and_wait_service(SC_HANDLE hServ,int wait_time_sec)
{
 int i;
 SERVICE_STATUS ServiceStatus;

 ServiceStatus.dwCurrentState=SERVICE_STOPPED;

 if (!StartService(hServ,0,NULL))
	 return FALSE;
 for (i=0;i<(wait_time_sec*2);i++)
 {
	 if (!QueryServiceStatus(hServ,&ServiceStatus))
		 return FALSE;
	 if (ServiceStatus.dwCurrentState!=SERVICE_START_PENDING)
		 break;
	 Sleep(500);
 }
 if (ServiceStatus.dwCurrentState!=SERVICE_RUNNING)
 {
	 SetLastError(ERROR_SERVICE_NOT_ACTIVE);
	 return FALSE;
 }
 return TRUE;
}

BOOL start_and_wait_service(SC_HANDLE hSCM,LPCTSTR svc_name,int wait_time_sec)
{
 BOOL b;
 SC_HANDLE hServ;
 hServ=OpenService(hSCM,svc_name,SERVICE_QUERY_CONFIG|SERVICE_QUERY_STATUS|SERVICE_START);
 if (!hServ) return FALSE;
 b=start_and_wait_service(hServ,wait_time_sec);
 DWORD dwErr = GetLastError();
 CloseServiceHandle(hServ);
 SetLastError(dwErr);
 return b;
}
BOOL start_and_wait_service(LPCTSTR svc_name,int wait_time_sec)
{
 BOOL b;
 SC_HANDLE hSCM;
 hSCM=OpenSCManagerW(NULL,NULL,0);
 if (!hSCM) return FALSE;
 b=start_and_wait_service(hSCM,svc_name,wait_time_sec);
 DWORD dwErr = GetLastError();
 CloseServiceHandle(hSCM);
 SetLastError(dwErr);
 return b;
}
