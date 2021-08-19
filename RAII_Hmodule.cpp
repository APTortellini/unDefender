#include "raii.h"

namespace RAII
{
	Hmodule::Hmodule(HMODULE inputHmodule)
	{
		_internalHmodule = inputHmodule;
	}

	Hmodule::~Hmodule()
	{
		::FreeLibrary(_internalHmodule);
	}

	HMODULE Hmodule::GetHmodule()
	{
		return _internalHmodule;
	}
}