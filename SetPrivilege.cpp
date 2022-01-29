#include "common.hpp"

bool SetPrivilege(
    _In_ HANDLE token, // won't be using RAII as we don't need the HANDLE to get closed after the function returns
    _In_ std::wstring privilege,
    _In_ bool enableDisable
)
{
    TOKEN_PRIVILEGES tp{};
    LUID luid;

    auto success = LookupPrivilegeValueW(NULL, privilege.c_str(), &luid);

    if (!success)
    {
        Error(GetLastError());
        std::wcout << L"[-] The call to LookupPrivilegeValueW failed...\n";
        return false;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;

    if (enableDisable)
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    else
        tp.Privileges[0].Attributes = 0;

    success = AdjustTokenPrivileges(token, false, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL);
    auto result = GetLastError(); // GetLastError tells us if the privilege has been successfully assigned
    if (result != ERROR_SUCCESS)
    {
        Error(GetLastError());
        std::wcout << L"[-] Couldn't add privilege " << privilege << L" to the current token...\n";
        return false;
    }
    std::wcout << L"[+] Privilege " << privilege << L" added to the current token!\n";
    return true;
}
