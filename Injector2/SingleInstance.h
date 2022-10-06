#pragma once

#include <Windows.h>

#include "XorString.h"

namespace single_instance
{
	inline auto test() -> bool
	{
		SetLastError(0);

		auto bCreate = CreateMutexA(NULL, TRUE, x("APP_ID_B234JHB6532342HJ35-2"));
		if (bCreate == NULL || GetLastError() == ERROR_ALREADY_EXISTS)
		{
			return false;
		}

		return true;
	}
};