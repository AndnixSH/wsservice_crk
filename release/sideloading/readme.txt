This folder is about making repacked or proprietary redistributable packages.
Microsoft calls them LOB (Line-Of-Bussiness) applications.
Installing LOB apps is called Sideloading.

By microsoft product policy enforced by sppsvc installation of LOBs
is possible only on 
 1) win8 Enterprise and server 2012.
    Computer must be either domain joined or have special sideloading SKU.
 2) Win8 Pro.
    Sideloading SKU must be activated. Domain join has no effect.

WSServiceCrk is capable of faking the fact that computer is joined to a domain
and thus allow sideloading.
Starting with v1.4.0 it also can fake some product policy values and enable
sideloading on any edition of win8/2012.

Alternative way to enable sideloading - use ProductPolicyEditor.
To do it you will have to disable Software Protection service
and maintain ProductPolicy manually.
Use ProductPolicyEditor to change to "1" policy values
 WSLicensingService-EnableLOBApps
 WSLicensingService-LOBSideloadingActivated.
With those values changed to "1" you can sideload your appx without WSServiceCrk.
