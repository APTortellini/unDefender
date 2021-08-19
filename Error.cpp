#include "common.h"

void Error(_In_ DWORD lastError)
{
    wchar_t buf[256];
    FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, lastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        buf, (sizeof(buf) / sizeof(wchar_t)), NULL);

    std::wcout << "[-] Error code: 0x" << std::hex << lastError << L". Error string: " << buf;
}