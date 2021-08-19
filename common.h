#pragma once
#pragma comment(lib,"ntdll.lib")
#include <Windows.h>
#include <winternl.h>
#include <ntstatus.h>
#include <TlHelp32.h>
#include <iostream>
#include <algorithm>
#include "raii.h"

#define SYMBOLIC_LINK_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | 0x1)

typedef NTSTATUS(NTAPI* pNtMakeTemporaryObject)
(
	_In_ HANDLE Handle
);

typedef NTSTATUS(NTAPI* pNtOpenSymbolicLinkObject)
(
	_Out_ PHANDLE            LinkHandle,
	_In_  ACCESS_MASK        DesiredAccess,
	_In_  POBJECT_ATTRIBUTES ObjectAttributes
);

typedef NTSTATUS (NTAPI* pNtCreateSymbolicLinkObject)
(
	_Out_ PHANDLE LinkHandle,
	_In_  ACCESS_MASK DesiredAccess,
	_In_  POBJECT_ATTRIBUTES ObjectAttributes,
	_In_  PUNICODE_STRING LinkTarget
);

typedef NTSTATUS (NTAPI* pNtImpersonateThread)
(
	_In_ HANDLE ServerThreadHandle,
	_In_ HANDLE ClientThreadHandle,
	_In_ PSECURITY_QUALITY_OF_SERVICE SecurityQos
);

typedef NTSTATUS (NTAPI* pNtUnloadDriver)
(
	_In_  PUNICODE_STRING DriverServiceName
);

NTSTATUS ChangeSymlink
(
	_In_ std::wstring symLinkName, 
	_In_ std::wstring target
);

void Error
(
	_In_ DWORD lastError
);

DWORD FindPID
(
	_In_ std::wstring imageName
);

DWORD GetFirstThreadID
(
	_In_ DWORD dwOwnerPID
);

bool SetPrivilege(
	_In_ HANDLE token,
	_In_ std::wstring privilege,
	_In_ bool enableDisable
);

NTSTATUS ImpersonateAndUnload();