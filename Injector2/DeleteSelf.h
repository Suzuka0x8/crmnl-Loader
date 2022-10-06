#pragma once

#include <Windows.h>
#include <strsafe.h>

#include "Handler.h"
#include "XorString.h"

#define SELF_REMOVE_STRING  x("cmd.exe /C ping 1.1.1.1 -n 1 -w 3000 > Nul & Del /f /q \"%s\"")

namespace del
{
	__forceinline auto safe() -> void
	{
		// Deinitialize driver before executing or else BSOD
		handler::deinit();
		std::this_thread::sleep_for(std::chrono::seconds(2));

		// Force exit returning 1
		exit(1);
	}

	__forceinline auto hard() -> void
	{
		// Deinitialize driver before executing or else BSOD
		handler::deinit();
		std::this_thread::sleep_for(std::chrono::seconds(2));

#ifdef DEBUG_MODE
		// Format the string to delete the self program
		TCHAR szModuleName[MAX_PATH];
		TCHAR szCmd[2 * MAX_PATH];
		STARTUPINFO si = { 0 };
		PROCESS_INFORMATION pi = { 0 };
		GetModuleFileName(NULL, szModuleName, MAX_PATH);
		StringCbPrintf(szCmd, 2 * MAX_PATH, SELF_REMOVE_STRING, szModuleName);
		CreateProcess(NULL, szCmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
#endif

		// Force exit returning 1
		exit(1);
	}
}