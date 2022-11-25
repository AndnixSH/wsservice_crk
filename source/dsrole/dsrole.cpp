// dsrole.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"


#include <dsrole.h>

extern "C" {
DWORD
WINAPI
DsRoleGetPrimaryDomainInformation(
    IN  LPCWSTR lpServer OPTIONAL,
    IN  DSROLE_PRIMARY_DOMAIN_INFO_LEVEL InfoLevel,
    OUT PBYTE *Buffer 
    )
{
	return 0;
}

VOID
WINAPI
DsRoleFreeMemory(
    IN PVOID    Buffer
    )
{
}

};
