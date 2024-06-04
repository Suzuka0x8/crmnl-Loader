#include <windows.h>
#include <stdio.h>
#include <cstdint>
#include <time.h>


int rand_large_range(int min, int max)
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
	HANDLE handle = CreateFile("C:/Windows/System32/winbioext.dll", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	FILETIME create_time;
	GetFileTime(handle, &create_time, 0, 0);
	CloseHandle(handle);

	return create_time.dwHighDateTime + create_time.dwLowDateTime;
}


int main()
{
	// rand mit dem install timesptamp versehen, damit Bastian sich selbst detected
	// dann denkt er das es fully detected ist haha
	// und laesst uns in Ruhe!!!!
	// idiotensichere Idee
	srand(install_stamp());
	//srand(time(0));

	std::uint64_t rwx_module = reinterpret_cast<std::uint64_t>(LoadLibraryA("testdll.dll"));
	printf("Loaded Module: 0x%llX\n", rwx_module);

	auto dos = *reinterpret_cast<IMAGE_DOS_HEADER*>(rwx_module);
	auto old_header = *reinterpret_cast<IMAGE_NT_HEADERS*>(rwx_module + dos.e_lfanew);
	printf("old_header: 0x%llX\n", old_header.FileHeader.TimeDateStamp);
	old_header.FileHeader.TimeDateStamp = rand_large_range(0x5B000000, 0x5F000000);

	DWORD old_protect;
	VirtualProtect((void*)(rwx_module + dos.e_lfanew), sizeof(IMAGE_NT_HEADERS), PAGE_EXECUTE_READWRITE, &old_protect);
	*reinterpret_cast<IMAGE_NT_HEADERS*>(rwx_module + dos.e_lfanew) = old_header;
	VirtualProtect((void*)(rwx_module + dos.e_lfanew), sizeof(IMAGE_FILE_HEADER), old_protect, 0x0);

	auto new_header = *reinterpret_cast<IMAGE_NT_HEADERS*>(rwx_module + dos.e_lfanew);
	printf("new_header: 0x%llX\n", new_header.FileHeader.TimeDateStamp);

	system("pause");
}
