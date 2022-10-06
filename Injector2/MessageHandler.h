#pragma once

#include <iostream>
#include <Windows.h>
#include <stdio.h>
#include <stdarg.h>

#include "XorString.h"

int filterException(int code, PEXCEPTION_POINTERS ex)
{
	Log(x("[!!] Filtering ") << std::hex << code << std::endl);
	return EXCEPTION_EXECUTE_HANDLER;
}

namespace message
{
	inline auto info(const char* msg, ...) -> void
	{
		va_list vl;
		va_start(vl, msg);
		char buff[1024];
		vsprintf_s(buff, msg, vl);

		MessageBoxA(0, buff, x("Information"), MB_OK | MB_ICONINFORMATION);
	}

	inline auto error(const char* msg, ...) -> void
	{
		va_list vl;
		va_start(vl, msg);
		char buff[1024];
		vsprintf_s(buff, msg, vl);

		MessageBoxA(0, buff, x("Error"), MB_OK | MB_ICONERROR);
	}
}