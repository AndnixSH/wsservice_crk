set DLL_PATH=%SystemRoot%\system32\wsservice_crk.dll
set SVC_NAME=WSServiceCrk
set SVC_DISPLAY_NAME=Windows Store Service crack
set DLL_START=ServiceMain


set SVC_GROUP=LocalServiceAndNoImpersonation
set SVC_KEY=HKLM\SYSTEM\CurrentControlSet\services\%SVC_NAME%\Parameters
set SVC_KEY_PARM=%SVC_KEY%\Parameters
sc delete %SVC_NAME%
sc create %SVC_NAME% binPath= "%SystemRoot%\system32\svchost.exe -k %SVC_GROUP%" type= share start= demand DisplayName= "%SVC_DISPLAY_NAME%" obj= "NT AUTHORITY\LocalService"
reg add %SVC_KEY% /t REG_EXPAND_SZ /v "ServiceDll" /d "%DLL_PATH%" /f 
reg add %SVC_KEY% /t REG_SZ /v "ServiceMain" /d "%DLL_START%" /f 
reg add %SVC_KEY% /t REG_DWORD /v "ServiceDllUnloadOnStop" /d 1 /f 

rem And add it to HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\svchost\LocalServiceAndNoImpersonation manually
