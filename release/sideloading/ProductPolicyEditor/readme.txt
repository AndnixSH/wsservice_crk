This packages is applicable to all windows editions starting from
Windows Vista and ending with Windows 8/Server 2012.

.NET Framework 4.0 is required.


Have you ever thought about the difference between Windows Server Datacenter
and Windows Starter Edition ? 
All windows editions are compiled from the single code base.
Some editions can have some components while others missing them.
But its not full truth. Even starter edition already has most of the components
needed to convert it to ultimate edition.

Most of features of windows are controlled by the single registry key :
HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\ProductOptions\ProductPolicy

ProductPolicy is protected by kernel - if you write the value - its content
remain unchanged although SetValue operation succeeds.
To overcome this protection you need to reboot in setup mode :
HKEY_LOCAL_MACHINE\SYSTEM\Setup
 SetupType=1
 CmdLine="cmd.exe"

You can change the values in this mode but they will be reverted back
by Software Protection service ("sppsvc") according to licensing templates
in C:\windows\system32\spp. They are signed and cannot be changed without
hacking of sppsvc.

I don't know what kind of operations require sppsvc running.
Most ops can be done with sppsvc disabled.

If you decided to modify ProductPolicy I suggest :

1) Disable sppsvc
2) Reboot in SetupMode
3) Change needed values
4) Save your changed policy to a file
5) Reboot in normal mode

Whenever you need to reenable sppsvc :

1) Do it
2) Do all you wanted to do with sppsvc enabled
3) Disable it back
4) Reboot in SetupMode
5) Reapply your changed policy from file
6) Reboot in normal mode


*** Kernel-WindowsMaxMemAllowedX64
maximum memory allowed on x64 system (in MB):

*** Kernel-WindowsMaxMemAllowedX86 + Kernel-MaxPhysicalPage
maximum memory allowed on x68 system (in MB)
set them both to 65536 to unlock PAE capabilities
You can have up to 64G of ram on 32-bit system !

*** WSLicensingService-EnableLOBApps+WSLicensingService-LOBSideloadingActivated
Set them both to 1 to enable sideloading of metro apps even on Win8 Core Edition

*** TerminalServices-RemoteConnectionManager-45344fe7-00e6-4ac6-9f01-d01fd4ffadfb-LocalOnly
1 = Do not allow remote RDP connections

*** TerminalServices-RemoteConnectionManager-45344fe7-00e6-4ac6-9f01-d01fd4ffadfb-MaxSessions
Terminal services mode.
0 = logon impossible even on console
1 = 1 remote connection. if a user is logged on locally he will be logged out.
 (workstation mode. default on win 7/win 8)
2 = possibility of simultaneous sessions  (server mode)
 (2 is default on server)

*** volmgrx-SupportRaid5
1 = Allow software raid-5 (disabled on win7/8, enabled on server)

*** WorkstationService-DomainJoinEnabled
1 = can be joined to domain. disabled on starter/home/core edition.


Dig, try, and you will find much more !


SIDE EFFECTS OF HAVING SPPSVC DISABLED
--------------------------------------

On win7 after ~3 hours of run you will notice some side effects.

1) When launching control panel you will see non-genuine nag dialog.
It can be dismissed with no consequences.
2) When starting windows update you will also see some nag but windows
update will work.
3) When logging in you will see SLUI reminder and background will be set
to black. This is the most anoying. To overcome this shit you can
take ownership of file C:\windows\system32\slui.exe, change perms and
rename it. This will prevent launch of slui on logon.
