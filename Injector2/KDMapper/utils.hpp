#pragma once

#include "../XorString.h"

//#define DISABLE_OUTPUT

#if defined(DISABLE_OUTPUT)
		#define Log(content)
#else
		#define Log(content) utils::out_file.open(x("log.txt"), std::ofstream::out | std::ofstream::app); utils::out_file.rdbuf()->pubsetbuf(0, 0); utils::out_file << content; utils::out_file.close()
#endif


#include <Windows.h>
#include <TlHelp32.h>
#include <stdint.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "nt.hpp"

namespace utils
{
	inline std::ofstream out_file;

	std::wstring GetFullTempPath();
	bool ReadFileToMemory(const std::wstring& file_path, std::vector<uint8_t>* out_buffer);
	bool CreateFileFromMemory(const std::wstring& desired_file_path, const char* address, size_t size);
	uint64_t GetKernelModuleAddress(const std::string& module_name);
	BOOLEAN bDataCompare(const BYTE* pData, const BYTE* bMask, const char* szMask);
	uintptr_t FindPattern(uintptr_t dwAddress, uintptr_t dwLen, BYTE* bMask, char* szMask);
	PVOID FindSection(char* sectionName, uintptr_t modulePtr, PULONG size);

	inline std::string wtos(std::wstring wstr)
	{
		return std::string(wstr.begin(), wstr.end());
	}

	inline atd::string wtos(wchar_t* wchar)
	{
		return wtos(std::wstring(wchar));
	}
}