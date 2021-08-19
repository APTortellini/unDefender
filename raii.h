#pragma once
#include "common.h"

namespace RAII
{
	class Handle
	{
	public:
		Handle(HANDLE inputHandle);
		~Handle();
		HANDLE GetHandle();

	private:
		HANDLE _internalHandle;
	};

	class Hmodule
	{
	public:
		Hmodule(HMODULE inputHmodule);
		~Hmodule();
		HMODULE GetHmodule();

	private:
		HMODULE _internalHmodule;
	};

	class ScHandle
	{
	public:
		ScHandle(SC_HANDLE inputHandle);
		~ScHandle();
		SC_HANDLE GetHandle();

	private:
		SC_HANDLE _internalHandle;
	};
};