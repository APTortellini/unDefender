#include "common.h"

DWORD GetFirstThreadID(_In_ DWORD dwOwnerPID)
{
    THREADENTRY32 te32;

    // Take a snapshot of all running threads  
    RAII::Handle hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hThreadSnap.GetHandle() == INVALID_HANDLE_VALUE) return 0;

    // Fill in the size of the structure before using it. 
    te32.dwSize = sizeof(THREADENTRY32);

    // Retrieve information about the first thread,
    Thread32First(hThreadSnap.GetHandle(), &te32);

    // Now walk the thread list of the system,
    do
    {
        if (te32.th32OwnerProcessID == dwOwnerPID)
        {
            return te32.th32ThreadID;
        }
    } while (Thread32Next(hThreadSnap.GetHandle(), &te32));

    return ERROR_FILE_NOT_FOUND;
}