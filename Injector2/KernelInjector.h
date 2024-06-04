#pragma once

#include <map>

#include "Handler.h"


#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

namespace injector
{
	int ends(const char* str, const char* suffix)
	{
		if (!str || !suffix)
			return 0;
		size_t lenstr = strlen(str);
		size_t lensuffix = strlen(suffix);
		if (lensuffix > lenstr)
			return 0;
		return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
	}

	inline void* alignBack(void* address, std::size_t alignment) noexcept
	{
		void* aligned = address;
		std::size_t space = alignment;
		std::align(alignment, 1, aligned, space);
		return address == aligned
			? aligned
			: static_cast<char*>(aligned) - alignment;
	}

	inline int rand_large_range(int min, int max)
	{
		int rand_num = min;
		int max_loop = (max - min) % RAND_MAX;
		for (auto idx = 0; idx < max_loop; idx++)
		{
			rand_num += rand() % (max - min);
		}
		return (rand_num > max) ? (max - (rand() % (max - min))) : rand_num;
	}

	int install_stamp()
	{
		HANDLE handle = CreateFileA(x("C:/Windows/System32/winbioext.dll"), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		FILETIME create_time;
		GetFileTime(handle, &create_time, 0, 0);
		CloseHandle(handle);

		return create_time.dwHighDateTime + create_time.dwLowDateTime;
	}

	inline DWORD randEntry(DWORD rwx_section_address, DWORD rwx_size, DWORD module_size)
	{
		DWORD max = (rwx_section_address + rwx_size) - module_size;
		DWORD min = rwx_section_address + 0x1000;
		DWORD random = rand_large_range(min, max);
		return (DWORD)alignBack((void*)random, 0x10);
	}

	__forceinline auto remoteCall(HWND window, std::uint64_t pointer, int num_call = 1)
	{
		for (auto idx = 0; idx < num_call; idx++)
		{
			Log(x("[>] Remote Calling Address") << std::endl);

			auto tid = GetWindowThreadProcessId(window, NULL);
			if (!tid)
			{
				Log(x("[-] Failed to get TID -> did the game close/crash early?") << std::endl);
				return;
			}

			auto wnhk = SetWindowsHookExA(WH_GETMESSAGE, (HOOKPROC)pointer, GetModuleHandleA(x("ntdll.dll")), tid);
			if (!PostThreadMessageA(tid, WM_USER + 400, 0, 0))
			{
				Log(x("[-] Failed to PTM") << std::endl);
			}

			std::this_thread::sleep_for(std::chrono::seconds(1));
			UnhookWindowsHookEx(wnhk);
		}
	}

	__forceinline auto loadRwx() -> std::uint64_t
	{
		Log(x("[>] Loading target module") << std::endl);

		std::uint8_t shellcode[] =
		{
			0x51,															// push    rcx
			0x52,															// push    rdx
			0x41, 0x50,														// push    r8

			0x48, 0x31, 0xC9,												// xor     rcx, rcx
			0x48, 0x31, 0xD2,												// xor     rdx, rdx
			0x4D, 0x31, 0xC0,												// xor     r8, r8

			0x48, 0x83, 0xEC, 0x20,											// sub     rsp, 20h

			0x41, 0xB8, 0xEF, 0xBE, 0xAD, 0xDE,								// mov     r8d, 0DEADBEEFh			; flags
			0x48, 0xB9, 0xEF, 0xBE, 0xFE, 0xCA, 0xEF, 0xBE, 0xAD, 0xDE,		// mov     rcx, 0DEADBEEFCAFEBEEFh	; name buffer address
			0x48, 0xB8, 0xEF, 0xBE, 0xFE, 0xCA, 0xEF, 0xBE, 0xAD, 0xDE,		// mov     rax, 0DEADBEEFCAFEBEEFh	; LoadLibraryExA address

			0xFF, 0xD0,														// call    rax

			0x48, 0x83, 0xC4, 0x20,											// add     rsp, 20h

			0x41, 0x58,														// pop     r8
			0x5A,															// pop     rdx
			0x59,															// pop     rcx

			0xC3															// ret
		};

		auto loadlib_flag = DONT_RESOLVE_DLL_REFERENCES;
		auto loadlib_name = x("grputil32.dll");
		auto loadlib_size = static_cast<std::uint32_t>(sizeof(shellcode) + sizeof(loadlib_name) + sizeof(std::uint64_t));

		if (loadlib_size < 0x1000)
			loadlib_size = 0x1000;

		auto loadlib_stub = handler::alloc(0x0, loadlib_size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		if (!loadlib_stub)
		{
			Log(x("[-] Could not allocate memory for the stub") << std::endl);
			return 0x0;
		}

		handler::write(loadlib_stub, 0, loadlib_size);

		handler::write(loadlib_stub, reinterpret_cast<std::uint64_t>(&shellcode), sizeof(shellcode));
		for (auto idx = 0; idx < strlen(loadlib_name); idx++) { handler::write<std::uint8_t>(loadlib_stub + sizeof(shellcode) + idx, loadlib_name[idx]); }

		handler::write<std::uint32_t>(loadlib_stub + 19, loadlib_flag);
		handler::write<std::uint64_t>(loadlib_stub + 25, loadlib_stub + sizeof(shellcode));
		handler::write<std::uint64_t>(loadlib_stub + 35, reinterpret_cast<std::uint64_t>(LoadLibraryExA));

		uint64_t rwx_module = 0x0;

		int attemptLoad = 10;
		while (attemptLoad)
		{
			remoteCall(FindWindowA((auth::windowClass.length() == 0) ? 0 : auth::windowClass.c_str(), (auth::windowName.length() == 0) ? 0 : auth::windowName.c_str()), loadlib_stub);
			std::this_thread::sleep_for(std::chrono::seconds(1));

			rwx_module = handler::module(x(L"grputil32.dll"));
			if (rwx_module)
			{
				break;
			}

			attemptLoad--;
		}

		std::this_thread::sleep_for(std::chrono::seconds(1));
		for (auto idx = 0; idx < loadlib_size; idx++) { handler::write<std::uint8_t>(loadlib_stub + idx, 0); }

		if (!attemptLoad && !rwx_module)
		{
			Log(x("[-] Failed to find the target module. Is default UAC options enforced?") << std::endl);
			return 0x0;
		}

		// random timestamp

		srand(install_stamp());

		auto dos = handler::read<IMAGE_DOS_HEADER>(rwx_module);
		auto old_header = handler::read<IMAGE_NT_HEADERS>(rwx_module + dos.e_lfanew);
		old_header.FileHeader.TimeDateStamp = rand_large_range(0x5B000000, 0x5F000000);
		auto old_protect = handler::protect(rwx_module + dos.e_lfanew, sizeof(IMAGE_NT_HEADERS), PAGE_EXECUTE_READWRITE);
		handler::write<IMAGE_NT_HEADERS>(rwx_module + dos.e_lfanew, old_header);
		handler::protect(rwx_module + dos.e_lfanew, sizeof(IMAGE_NT_HEADERS), old_protect);

		// end random timestamp

		return rwx_module;
	}

	inline std::map<std::string, uint64_t> imports;
	__forceinline auto parse_imports() -> bool
	{
		Log(x("[>] Parsing imports") << std::endl);

		auto base{ handler::baseAddress };
		if (!base)
		{
			Log(x("[-] Could not find module base when parsing imports") << std::endl);
			return false;
		}

		auto dos_header{ handler::read< IMAGE_DOS_HEADER >(base) };
		auto nt_headers{ handler::read< IMAGE_NT_HEADERS >(base + dos_header.e_lfanew) };
		auto descriptor{ handler::read< IMAGE_IMPORT_DESCRIPTOR >(base + nt_headers.OptionalHeader.DataDirectory[1].VirtualAddress) };

		int descriptor_count{ 0 };
		int thunk_count{ 0 };

		while (descriptor.Name)
		{
			auto first_thunk{ handler::read< IMAGE_THUNK_DATA >(base + descriptor.FirstThunk) };
			auto original_first_thunk{ handler::read< IMAGE_THUNK_DATA >(base + descriptor.OriginalFirstThunk) };
			thunk_count = 0;

			while (original_first_thunk.u1.AddressOfData)
			{
				char name[256];
				handler::read(base + original_first_thunk.u1.AddressOfData + 0x2, (uintptr_t)name, 256);
				std::string str_name(name);
				auto thunk_offset{ thunk_count * sizeof(uintptr_t) };

				if (str_name.length() > 0)
				{
					imports[str_name] = base + descriptor.FirstThunk + thunk_offset;
				}

				++thunk_count;
				first_thunk = handler::read< IMAGE_THUNK_DATA >(base + descriptor.FirstThunk + sizeof(IMAGE_THUNK_DATA) * thunk_count);
				original_first_thunk = handler::read< IMAGE_THUNK_DATA >(base + descriptor.OriginalFirstThunk + sizeof(IMAGE_THUNK_DATA) * thunk_count);
			}

			++descriptor_count;
			descriptor = handler::read< IMAGE_IMPORT_DESCRIPTOR >(base + nt_headers.OptionalHeader.DataDirectory[1].VirtualAddress + sizeof(IMAGE_IMPORT_DESCRIPTOR) * descriptor_count);
		}

		return (imports.size() > 0);
	}

	__forceinline auto get_proc_address(const char* module_name, const char* func) -> uint64_t
	{
		Log(x("[>] Getting function address") << std::endl);

		SetLastError(0);

		HMODULE hmod = LoadLibraryA(module_name);
		if (!hmod)
		{
			if (ends(module_name, x(".dll")))
			{
				if (module_name == x("d3d11.dll"))
				{
					Log(x("[-] could not find module ") << module_name << x(" gle: ") << GetLastError() << std::endl);
					Log(x("[-] manually resolving d3d11.dll") << std::endl);

					LoadLibraryA(x("d3d11.dll"));
					return reinterpret_cast<uint64_t>(D3D11CreateDeviceAndSwapChain);
				}

				Log(x("[-] Module ends in .dll -> could not find module ") << module_name << x(" gle: ") << GetLastError() << std::endl);
				return 0;
			}
			else
			{
				std::string dllname = module_name;
				dllname += x(".dll");
				hmod = LoadLibraryA(dllname.c_str());
				if (!hmod)
				{
					Log(x("[-] Could not find the module when resolving ") << module_name << x(" gle: ") << GetLastError() << std::endl);
					return 0;
				}
			}
		}

		uint64_t funcadd = (uint64_t)GetProcAddress(hmod, func);
		if (!funcadd)
		{
			Log(x("[-] Could not find the desired function address") << x(" gle: ") << GetLastError() << std::endl);
			return 0;
		}
		return funcadd;
	}

	__forceinline auto get_section_header(uint64_t rva, PIMAGE_NT_HEADERS nt_header) -> PIMAGE_SECTION_HEADER
	{
		Log(x("[>] Getting section header") << std::endl);

		PIMAGE_SECTION_HEADER section{ IMAGE_FIRST_SECTION(nt_header) };
		for (int i = 0; i < nt_header->FileHeader.NumberOfSections; i++, section++)
		{
			uint64_t size{ section->Misc.VirtualSize };
			if (!size)
				size = section->SizeOfRawData;

			if ((rva >= section->VirtualAddress) &&
				(rva < (section->VirtualAddress + size)))
			{
				return section;
			}
		}

		Log(x("[-] Failed to find the section header") << std::endl);
		return 0;
	}

	__forceinline auto get_ptr_from_rva(uint64_t rva, IMAGE_NT_HEADERS* nt_header, uint8_t* image_base) -> uint64_t*
	{
		Log(x("[>] Getting pointer from RVA") << std::endl);

		PIMAGE_SECTION_HEADER section_header{ get_section_header(rva, nt_header) };
		if (!section_header)
			return 0;

		int64_t delta{ (int64_t)(section_header->VirtualAddress - section_header->PointerToRawData) };

		return (uint64_t*)(image_base + rva - delta);
	}

	__forceinline auto solve_relocations(uint64_t base, uint64_t relocation_base, IMAGE_NT_HEADERS* nt_header, IMAGE_BASE_RELOCATION* reloc, size_t size)
	{
		Log(x("[SR] Solving relocations") << std::endl);

		uint64_t image_base{ nt_header->OptionalHeader.ImageBase };
		uint64_t delta{ relocation_base - image_base };
		unsigned int bytes{ 0 };

		while (bytes < size)
		{
			uint64_t* reloc_base{ (uint64_t*)get_ptr_from_rva((uint64_t)(reloc->VirtualAddress), nt_header, (PBYTE)base) };
			auto num_of_relocations{ (reloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD) };
			auto reloc_data = (uint16_t*)((uint64_t)reloc + sizeof(IMAGE_BASE_RELOCATION));

			for (unsigned int i = 0; i < num_of_relocations; i++)
			{
				if (((*reloc_data >> 12) & IMAGE_REL_BASED_HIGHLOW))
					*(uint64_t*)((uint64_t)reloc_base + ((uint64_t)(*reloc_data & 0x0FFF))) += delta;
				reloc_data++;
			}

			bytes += reloc->SizeOfBlock;
			reloc = (IMAGE_BASE_RELOCATION*)reloc_data;
		}
	}

	__forceinline auto solve_imports(uint8_t* base, IMAGE_NT_HEADERS* nt_header, IMAGE_IMPORT_DESCRIPTOR* import_descriptor)
	{
		Log(x("[SI] Solving imports") << std::endl);

		char* _module;
		while ((_module = (char*)get_ptr_from_rva((DWORD64)(import_descriptor->Name), nt_header, (PBYTE)base)))
		{
			Log(x("[SI] Current Module -> ") << _module << std::endl);

			IMAGE_THUNK_DATA* thunk_data{ (IMAGE_THUNK_DATA*)get_ptr_from_rva((DWORD64)(import_descriptor->FirstThunk), nt_header, (PBYTE)base) };

			while (thunk_data->u1.AddressOfData)
			{
				IMAGE_IMPORT_BY_NAME* iibn{ (IMAGE_IMPORT_BY_NAME*)get_ptr_from_rva((DWORD64)((thunk_data->u1.AddressOfData)), nt_header, (PBYTE)base) };
				uint64_t func = (uint64_t)(get_proc_address(_module, (char*)iibn->Name));

				Log(x("[SI] ImportBN -> ") << (char*)iibn->Name << std::endl);
				Log(x("[SI] Function -> 0x") << std::hex << func << std::endl);

				thunk_data->u1.Function = (uint64_t)(get_proc_address(_module, (char*)iibn->Name));
				thunk_data++;
			}
			import_descriptor++;
		}
		Log(x("[SI] Done") << std::endl);
		return;
	}

	__forceinline auto injectNew() -> bool
	{
		std::uint64_t base = loadRwx();
		if (!base)
		{
			Log(x("[-] Failed to load target module") << std::endl);
			return false;
		}

		base += randEntry(0x00502000, 0x0007BC3D + 0x00C00AC, 1000000);//Approx 1mb since your dll is 700kb and we cant use fix size

		std::uint64_t cheatBuffer; DWORD cheatSize;
		klar_cheat_auth::download_relocated(auth::fileID, &cheatBuffer, &cheatSize, base);
		if (!cheatBuffer || cheatSize < 0x2000)
		{
			Log(x("[>] Buffer2 SR Failed -> Server connection invalid? [] ") << std::hex << cheatSize << std::endl);

			message::error(x("Server connection timed out!\nIf problems persist send C:\\Windows\\System32\\log.txt to support"));
			return false;
		}

		Log(x("[>] Inject SR") << std::endl);

		//This looks horrible but works dont ask
		uintptr_t addr = (uintptr_t)cheatBuffer;
		while (true)
		{
			Log(x("[>] Resolving Loop Start") << std::endl);
			char libname[100] = { 0 };
			if (!strlen((char*)addr))
			{
				Log(x("[-] Length failed") << std::endl);
				break;
			}
			Log(x("[>] Copying name") << std::endl);
			strcpy_s(libname, (char*)addr);
			addr += strlen((char*)addr) + 1;
			while (true)
			{
				Log(x("[>] Start!") << std::endl);
				int len = strlen((char*)addr);
				DWORD offset;
				char funcname[100] = { 0 };
				char zero[4] = { 0,0,0,0 };
				Log(x("[>] Comparing mem") << std::endl);
				if (!memcmp((void*)addr, zero, 4))
				{
					Log(x("[-] Compare failed") << std::endl);
					break;
				}
				if (len <= 4)
				{
					Log(x("[>] Resolving Import") << std::endl);
					offset = *(DWORD*)addr;
					addr += sizeof(DWORD);
					Log(x("[>] 1") << std::endl);
					strcpy_s(funcname, (char*)addr);
					Log(x("[>] 2") << std::endl);
					addr += strlen((char*)addr) + 1;
					Log(x("[>] 3") << std::endl);
					HMODULE test1 = LoadLibraryA(std::string(libname).c_str());
					Log(x("[>] 3.5") << std::endl);
					Log(x("[>] 4 [] ") << std::string(funcname).c_str() << std::endl);
					uintptr_t funcaddy = (uintptr_t)GetProcAddress(test1, std::string(funcname).c_str());
					Log(x("[>] 5 [] ") << funcaddy << std::endl);
					*(uintptr_t*)(cheatBuffer + 0x5000 + offset) = funcaddy;
					Log(x("[>] RI EOL") << funcaddy << std::endl);
				}
				else
				{
					Log(x("[>] Breakout") << std::endl);
					break;
				}
				Log(x("[>] S! EOL") << std::endl);
			}
			Log(x("[>] RI EOL") << std::endl);
		}
		cheatBuffer += 0x5000;

		Log(x("[>] Obtaining pointer information") << std::endl);
		DWORD entry_rva = *(DWORD*)cheatBuffer;
		DWORD img_size = *(DWORD*)(cheatBuffer + sizeof(DWORD));

		uint64_t entry_point = base + entry_rva;

		//Zero rwx buffer
		Log(x("[>] 0RB") << std::endl);
		BYTE* zerobuffer = new BYTE[img_size];
		memset(zerobuffer, 0, img_size);
		handler::write(base, (uintptr_t)zerobuffer, img_size);

		struct section_data
		{
			DWORD virtual_address;
			DWORD pointer_to_raw_data;
			DWORD size_of_raw_data;
		};

		section_data* curr_sec = (section_data*)(cheatBuffer + sizeof(DWORD) * 2);
		while (curr_sec->virtual_address)
		{
			Log(x("[>] Writing CBS") << std::endl);
			handler::write(base + curr_sec->virtual_address, (uintptr_t)(cheatBuffer + curr_sec->pointer_to_raw_data), curr_sec->size_of_raw_data);
			curr_sec++;
		}

		static uint8_t dllmain_64[] =
		{
			0x48, 0x83, 0xEC, 0x28, 0x48, 0xB9, 0x64, 0x93, 0xAC, 0x1E, 0x62, 0x00, 0x00, 0x00, 0x48, 0x31, 0xD2, 0x48, 0x83, 0xC2, 0x01, 0x48, 0xB8, 0x34, 0x84, 0x11, 0x93, 0xC3, 0x04, 0x00, 0x00, 0xFF, 0xD0, 0x48, 0x83, 0xC4, 0x28,
			0x48, 0xB8 , 0x25 , 0x53 , 0x32 , 0x85 , 0x29 , 0x53 , 0x00 , 0x00 , 0xC7 , 0x00 , 0xC3 , 0x00 , 0x00 , 0x00,
			0xC3
		};

		auto dllmain_size = sizeof(dllmain_64);

		if (dllmain_size < 0x1000)
			dllmain_size = 0x1000;

		Log(x("[>] Allocating memory for stub") << std::endl);
		auto dllmain_stub = handler::alloc(0x0, dllmain_size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		if (!dllmain_stub)
		{
			Log(x("[-] Main stub allocation failed!") << std::endl);
			return false;
		}

		Log(x("[>] Modifying stub") << std::endl);
		*(uintptr_t*)(&dllmain_64[6]) = base;
		*(uintptr_t*)(&dllmain_64[23]) = entry_point;
		*(uintptr_t*)(&dllmain_64[0x27]) = dllmain_stub;

		Log(x("[>] Cleaning stub") << std::endl);
		for (auto idx = 0; idx < dllmain_size; idx++) { handler::write<std::uint8_t>(dllmain_stub + idx, 0); }

		Log(x("[>] Copying stub") << std::endl);
		handler::write(dllmain_stub, reinterpret_cast<std::uint64_t>(&dllmain_64), sizeof(dllmain_64));

		Log(x("[>] Calling stub") << std::endl);
		remoteCall(FindWindowA((auth::windowClass.length() == 0) ? 0 : auth::windowClass.c_str(), (auth::windowName.length() == 0) ? 0 : auth::windowName.c_str()), dllmain_stub, 5);
		std::this_thread::sleep_for(std::chrono::seconds(2));

		Log(x("[>] Cleaning stub") << std::endl);
		for (auto idx = 1; idx < dllmain_size; idx++) { handler::write<std::uint8_t>(dllmain_stub + idx, 0); }

		Log(x("[>] Done") << std::endl);
		return true;
	}

	__forceinline auto injectOld(std::uint64_t buffer_address) -> bool
	{
		Log(x("[>] Inject nSR") << std::endl);

		auto raw_data = (std::uint8_t*)buffer_address;
		IMAGE_DOS_HEADER* dos_header;
		IMAGE_NT_HEADERS* nt_header;

		Log(x("[>] Checking Raw Data") << std::endl);
		if (!raw_data)
		{
			Log(x("[-] Module data seems to be corrupted") << std::endl);
			return false;
		}

		Log(x("[>] Checking DOS Signature") << std::endl);
		dos_header = (IMAGE_DOS_HEADER*)raw_data;
		if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
		{
			Log(x("[-] No magic? Module data seems to be corrupted") << std::endl);
			return false;
		}

		Log(x("[>] Checking NT Signature") << std::endl);
		nt_header = (IMAGE_NT_HEADERS*)(&raw_data[dos_header->e_lfanew]);
		if (nt_header->Signature != IMAGE_NT_SIGNATURE)
		{
			Log(x("[-] No signature? Module data seems to be corrupted") << std::endl);
			return false;
		}

		Log(x("[>] Loading Target") << std::endl);
		// auto base = handler::alloc(0x0, nt_header->OptionalHeader.SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		std::uint64_t base = loadRwx();
		if (!base)
		{
			Log(x("[-] Failed to load target module") << std::endl);
			return false;
		}

		base += randEntry(0x00502000, 0x0007BC3D + 0x00C00AC, nt_header->OptionalHeader.SizeOfImage);

		Log(x("[>] Checking size") << std::endl);
		if (nt_header->OptionalHeader.SizeOfImage > 0x640000)
		{
			Log(x("[-] SizeOI test has failed!") << std::endl);
			return false;
		}
		else
		{
			Log(x("[>] Size passed -> Copying buffer") << std::endl);

			BYTE* buffer = new BYTE[nt_header->OptionalHeader.SizeOfImage];
			memset(buffer, 0, nt_header->OptionalHeader.SizeOfImage);
			handler::write(base, (uintptr_t)buffer, nt_header->OptionalHeader.SizeOfImage);
		}

		Log(x("[>] Parsing import descriptor") << std::endl);
		PIMAGE_IMPORT_DESCRIPTOR import_descriptor{ (PIMAGE_IMPORT_DESCRIPTOR)get_ptr_from_rva(
													(uint64_t)(nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress),
													nt_header,
													raw_data) };

		if (nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size)
		{
			solve_imports(raw_data, nt_header, import_descriptor);
		}

		Log(x("[>] Base Relocation") << std::endl);
		PIMAGE_BASE_RELOCATION base_relocation{ (PIMAGE_BASE_RELOCATION)get_ptr_from_rva(
																			nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress,
																			nt_header,
																			raw_data) };

		if (nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size)
		{
			solve_relocations((uint64_t)raw_data,
				base,
				nt_header,
				base_relocation,
				nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size);
		}

		// map pe sections
		auto header{ IMAGE_FIRST_SECTION(nt_header) };
		size_t virtual_size{ 0 };
		size_t bytes{ 0 };

		Log(x("[>] Mapping sections") << std::endl);
		while (nt_header->FileHeader.NumberOfSections && (bytes < nt_header->OptionalHeader.SizeOfImage))
		{
			handler::write(base + header->VirtualAddress, (uintptr_t)(raw_data + header->PointerToRawData), header->SizeOfRawData);

			if (header->VirtualAddress + header->SizeOfRawData > 0x640000)
			{
				Log(x("[-] SizeORD test has failed!") << std::endl);
				return false;
			}

			virtual_size = header->VirtualAddress;
			virtual_size = (++header)->VirtualAddress - virtual_size;
			bytes += virtual_size;
		}

		uint64_t entry_point = base + nt_header->OptionalHeader.AddressOfEntryPoint;

		static uint8_t dllmain_64[] =
		{
			0x48, 0x83, 0xEC, 0x28, 0x48, 0xB9, 0x64, 0x93, 0xAC, 0x1E, 0x62, 0x00, 0x00, 0x00, 0x48, 0x31, 0xD2, 0x48, 0x83, 0xC2, 0x01, 0x48, 0xB8, 0x34, 0x84, 0x11, 0x93, 0xC3, 0x04, 0x00, 0x00, 0xFF, 0xD0, 0x48, 0x83, 0xC4, 0x28,
			0x48, 0xB8 , 0x25 , 0x53 , 0x32 , 0x85 , 0x29 , 0x53 , 0x00 , 0x00 , 0xC7 , 0x00 , 0xC3 , 0x00 , 0x00 , 0x00,
			0xC3
		};

		auto dllmain_size = sizeof(dllmain_64);

		if (dllmain_size < 0x1000)
			dllmain_size = 0x1000;

		Log(x("[>] Allocating memory for stub") << std::endl);
		auto dllmain_stub = handler::alloc(0x0, dllmain_size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		if (!dllmain_stub)
		{
			Log(x("[-] Main stub allocation failed!") << std::endl);
			return false;
		}

		Log(x("[>] Modifying stub") << std::endl);
		*(uintptr_t*)(&dllmain_64[6]) = base;
		*(uintptr_t*)(&dllmain_64[23]) = entry_point;
		*(uintptr_t*)(&dllmain_64[0x27]) = dllmain_stub;

		Log(x("[>] Cleaning stub") << std::endl);
		for (auto idx = 0; idx < dllmain_size; idx++) { handler::write<std::uint8_t>(dllmain_stub + idx, 0); }

		Log(x("[>] Copying stub") << std::endl);
		handler::write(dllmain_stub, reinterpret_cast<std::uint64_t>(&dllmain_64), sizeof(dllmain_64));

		Log(x("[>] Calling stub") << std::endl);
		remoteCall(FindWindowA((auth::windowClass.length() == 0) ? 0 : auth::windowClass.c_str(), (auth::windowName.length() == 0) ? 0 : auth::windowName.c_str()), dllmain_stub, 5);
		std::this_thread::sleep_for(std::chrono::seconds(2));

		Log(x("[>] Cleaning stub") << std::endl);
		for (auto idx = 1; idx < dllmain_size; idx++) { handler::write<std::uint8_t>(dllmain_stub + idx, 0); }

		Log(x("[>] Done") << std::endl);
		return true;
	}
}

