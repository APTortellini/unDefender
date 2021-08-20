#include "common.h"

std::wstring GetSymbolicLinkTarget(_In_ std::wstring symLinkName)
{
    RAII::Hmodule ntdllModule = LoadLibraryW(L"ntdll.dll");
    if (!ntdllModule.GetHmodule())
    {
        std::cout << "[-] Couldn't open a handle to ntdll! Error: 0x" << std::hex << GetLastError() << std::endl;
        return L"";
    }
    pNtOpenSymbolicLinkObject NtOpenSymbolicLinkObject = (pNtOpenSymbolicLinkObject)GetProcAddress(ntdllModule.GetHmodule(), "NtOpenSymbolicLinkObject");
    if (!NtOpenSymbolicLinkObject)
    {
        std::cout << "[-] Couldn't resolve NtOpenSymbolicLinkObject address! Error: 0x" << std::hex << GetLastError() << std::endl;
        return L"";
    }
    pNtQuerySymbolicLinkObject NtQuerySymbolicLinkObject = (pNtQuerySymbolicLinkObject)GetProcAddress(ntdllModule.GetHmodule(), "NtQuerySymbolicLinkObject");
    if (!NtQuerySymbolicLinkObject)
    {
        std::cout << "[-] Couldn't resolve NtQuerySymbolicLinkObject address! Error: 0x" << std::hex << GetLastError() << std::endl;
        return L"";
    }

    // build OBJECT_ATTRIBUTES structure to reference and then delete the permanent symbolic link to the driver path
    UNICODE_STRING symlinkPath;
    RtlInitUnicodeString(&symlinkPath, symLinkName.c_str());
    OBJECT_ATTRIBUTES symlinkObjAttr{};
    InitializeObjectAttributes(&symlinkObjAttr, &symlinkPath, OBJ_KERNEL_HANDLE, NULL, NULL);
    HANDLE tempSymLinkHandle;

    NTSTATUS status = NtOpenSymbolicLinkObject(&tempSymLinkHandle, GENERIC_READ, &symlinkObjAttr);
    RAII::Handle symLinkHandle = tempSymLinkHandle;

    UNICODE_STRING LinkTarget{};
    wchar_t buffer[MAX_PATH] = { L'\0' };
    LinkTarget.Buffer = buffer;
    LinkTarget.Length = 0;
    LinkTarget.MaximumLength = MAX_PATH;

    status = NtQuerySymbolicLinkObject(symLinkHandle.GetHandle(), &LinkTarget, nullptr);
    if (!NT_SUCCESS(status))
    {
        Error(RtlNtStatusToDosError(status));
        std::wcout << L"[-] Couldn't get the target of the symbolic link " << symLinkName << std::endl;
        return L"";
    }
    else std::wcout << "[+] Symbolic link target is: " << LinkTarget.Buffer << std::endl;
    return LinkTarget.Buffer;
}