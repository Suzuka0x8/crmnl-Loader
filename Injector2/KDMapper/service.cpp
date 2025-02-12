#include "service.hpp"

bool service::RegisterAndStart(const std::wstring& driver_path) {
    const static DWORD ServiceTypeKernel = 1;
    const std::wstring driver_name = intel_driver::GetDriverNameW();
    const std::wstring servicesPath = L"SYSTEM\\CurrentControlSet\\Services\\" + driver_name;
    const std::wstring nPath = L"\\??\\" + driver_path;

    HKEY dservice;
    LSTATUS status = RegCreateKeyW(HKEY_LOCAL_MACHINE, servicesPath.c_str(), &dservice); // returns OK if already exists
    if (status != ERROR_SUCCESS) {
        Log(x("[-] Cant create service key") << std::endl);
        return false;
    }

    status = RegSetKeyValueW(dservice, NULL, L"ImagePath", REG_EXPAND_SZ, nPath.c_str(), (DWORD)(nPath.size()*sizeof(wchar_t)));
    if (status != ERROR_SUCCESS) {
        RegCloseKey(dservice);
        Log(x("[-] Cant create 'ImagePath' registry value") << std::endl);
        return false;
    }

    status = RegSetKeyValueW(dservice, NULL, L"Type", REG_DWORD, &ServiceTypeKernel, sizeof(DWORD));
    if (status != ERROR_SUCCESS) {
        RegCloseKey(dservice);
        Log(x("[-] Cant create 'Type' registry value") << std::endl);
        return false;
    }

    RegCloseKey(dservice);

    HMODULE ntdll = GetModuleHandleA("ntdll.dll");
    if (ntdll = NULL) {
        return false;
    }

    auto RtlAdjustPrivilege = (nt::RtlAdjustPrivilege)GetProcAddress(ntdll, "RtlAdjustPrivilege");
    auto NtLoadDriver = (nt::NtLoadDriver)GetProcAddress(ntdll, "NtLoadDriver");

    ULONG SE_LOAD_DRIVER_PRIVILEGE = 10UL;
    BOOLEAN SeLoadDriverWasEnabled;
    NTSTATUS Status = RtlAdjustPrivilege(SE_LOAD_DRIVER_PRIVILEGE, TRUE, FALSE, &SeLoadDriverWasEnabled);
    if (!NT_SUCCESS(Status)) {
        Log(x("Fatal error: failed to acquire SE_LOAD_DRIVER_PRIVILEGE. Make sure you are running as administrator.") << std::endl);
        return false;
    }

    std::wstring wdriver_reg_path = L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\" + driver_name;
    UNICODE_STRING serviceStr;
    RtlInitUnicodeString(&serviceStr, wdriver_reg_path.c_str());

    Status = NtLoadDriver(&serviceStr);
    Log(x("[+] NtLoadDriver Status 0x") << std::hex << Status << std::endl);

    // never should occur since kdmapper checks for "IsRunning" driver before
    if (Status == 0xC000010E) {// STATUS_IMAGE_ALREADY_LOADED
        return true;
    }

    // Check for FaceIT Anti-Cheat/Vanguard Anti-Cheat/Others
    if (Status == 0xC000009A || Status == 0xC0000022) {
        Log(x("Fatal error: failed to run NtLoadDriver due to FaceIT/Other Blocking Programs") << std::endl);
        return false;
    }

    return NT_SUCCESS(Status);
}

bool service::StopAndRemove(const std::wstring& driver_name) {
    HMODULE ntdll = GetModuleHandleA("ntdll.dll");
    if (ntdll == NULL)
        return false;

    std::wstring wdriver_reg_path = L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\" + driver_name;
    UNICODE_STRING serviceStr;
    RtlInitUnicodeString(&serviceStr, wdriver_reg_path.c_str());

    HKEY driver_service;
    std::wstring servicesPath = L"SYSTEM\\CurrentControlSet\\Services\\" + driver_name;
    LSTATUS status = RegOpenKeyW(HKEY_LOCAL_MACHINE, servicesPath.c_str(), &driver_service);
    if (status != ERROR_SUCCESS) {
        if (status == ERROR_FILE_NOT_FOUND) {
            return true;
        }
        return false;
    }
    RegCloseKey(driver_service);

    auto NtUnloadDriver = (nt::NtUnloadDriver)GetProcAddress(ntdll, "NtUnloadDriver");
    NTSTATUS st = NtUnloadDriver(&serviceStr);
    Log(x("[+] NtUnloadDriver Status 0x") << std::hex << st << std::endl);
    if (st != 0x0) {
        Log(x("[-] Driver Unload Failed!!") << std::endl);
        status = RegDeleteKeyW(HKEY_LOCAL_MACHINE, servicesPath.c_str());
        return false; // lets consider unload fail as error because can cause problems with anti cheats later
    }


    status = RegDeleteKeyW(HKEY_LOCAL_MACHINE, servicesPath.c_str());
    if (status != ERROR_SUCCESS) {
        return false;
    }
    return true;
}