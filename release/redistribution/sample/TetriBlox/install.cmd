@echo off
set LICFILE=TetriBlox.lic
set APPXFILE=TetriBlox.appx
set DEP=-dependency Microsoft.WinJS.1.0.appx

set POWERSHELL=%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe

"%POWERSHELL%" add-appxpackage %APPXFILE% %DEP%
if errorlevel 1 (
 echo appx install failed
 goto :ex
)

if %PROCESSOR_ARCHITECTURE%==AMD64 (
 %~dp0/wscrack_64/installer.exe inst
) ELSE (
 %~dp0/wscrack_32/installer.exe inst
)

if errorlevel 1 (
 echo WSCrack install failed
 goto :ex
)

echo importing license
wsll %LICFILE%

:ex
pause