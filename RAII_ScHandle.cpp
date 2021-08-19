//RAII::ScHandle methods definitions

#include "raii.h"

namespace RAII
{
	ScHandle::ScHandle(SC_HANDLE inputHandle)
	{
		_internalHandle = inputHandle;
	}

	ScHandle::~ScHandle()
	{
		::CloseHandle(_internalHandle);
	}

	SC_HANDLE ScHandle::GetHandle()
	{
		return _internalHandle;
	}
}