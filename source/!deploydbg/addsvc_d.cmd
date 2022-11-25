set DLL_PATH=%~dp0..\x64\debug\wsservice_crk.dll
set SVC_NAME=WSServiceCrkD
set SVC_DISPLAY_NAME=Windows Store Service crack (test)
set DLL_START=ServiceMain


set SVC_GROUP=%SVC_NAME%Group
set SVC_KEY=HKLM\SYSTEM\CurrentControlSet\services\%SVC_NAME%\Parameters
set SVC_KEY_PARM=%SVC_KEY%\Parameters
sc delete %SVC_NAME%
sc create %SVC_NAME% binPath= "%SystemRoot%\system32\svchost.exe -k %SVC_GROUP%" type= share start= demand DisplayName= "%SVC_DISPLAY_NAME%"
reg add "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\svchost" /t REG_MULTI_SZ /v "%SVC_GROUP%" /d "%SVC_NAME%" /f 
reg add %SVC_KEY% /t REG_EXPAND_SZ /v "ServiceDll" /d "%DLL_PATH%" /f 
reg add %SVC_KEY% /t REG_SZ /v "ServiceMain" /d "%DLL_START%" /f 
reg add %SVC_KEY% /t REG_DWORD /v "ServiceDllUnloadOnStop" /d 1 /f 
