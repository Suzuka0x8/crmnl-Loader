#pragma once

#include <vector>
#include <thread>

#include "./KDMapper/kdmapper.hpp"
#include "../CommData.h"

#include "MessageHandler.h"
#include "crmnl-CheatAuth.h"

namespace handler
{
	inline CommData::Port* pPort = nullptr;

	inline std::uint32_t processID = 0;
	inline std::uint64_t baseAddress = 0x0;
	inline std::uint64_t moduleAddress = 0x0;

	inline bool isInitialized = false;

	inline auto buildPort() -> std::uint64_t
	{
		// initialize the PortData structure and set it to 0
		CommData::Port PortData = {};
		memset(&PortData, 0, sizeof(CommData::Port));

		// Ready the PortData structure
		PortData.MagicValue = 0x1337DEADull;
		PortData.bAlive = 1ull;

		// Allocate the shared memory buffer
		void* PortAddress = malloc(sizeof(CommData::Port));
		//void* PortAddress = (void*)(__readgsqword(0x60) + 0x900);

		// Move the PortData structure to the allocation
		memcpy(PortAddress, &PortData, sizeof(CommData::Port));

		pPort = reinterpret_cast<CommData::Port*>(PortAddress);

		return reinterpret_cast<std::uint64_t>(PortAddress);
	}

	inline auto mapDriver(std::vector<std::uint8_t>* driverResource) -> bool
	{
		Log(x("------------------------------ map begin ------------------------------") << std::endl);

		std::uint64_t portAddress = buildPort();
		if (!portAddress)
		{
			message::error(x("Unexcepted error has occured! Please reboot and try again.\nIf problems still persist send C:\\Windows\\System32\\log.txt to support"));
			Log(x("---------------- map end failure (port buffer) ----------------") << std::endl);
			return false;
		}
		Log(x("[+] Port address at 0x") << std::hex << portAddress << std::endl);

		HANDLE intelHandle = intel_driver::Load();
		if (intelHandle == INVALID_HANDLE_VALUE)
		{
			message::error(x("Unexpected error has occured! Please reboot and try again.\nPlease very that FaceIT is not installed and reboot!\nIf problems persist send C:\\Windows\\System32\\log.txt to support"));
			Log(x("--------------------- map end failure ---------------------") << std::endl);
			return false;
		}

		if (!kdmapper::MapDriver(intelHandle, driverResource, portAddress, GetCurrentProcessId(), false, true, true))
		{
			intel_driver::Unload(intelHandle);

			message::error(x("Unexpected error has occured! Please reboot and try again.\nIf problems persist send C:\\Windows\\System32\\log.txt to support"));
			Log(x("--------------------- map end failure ---------------------") << std::endl);
			return false;
		}

		if (!intel_driver::Unload(intelHandle))
		{
			message::error(x("Unexpected error has occured! Please reboot and try again.\nIf problems persist send C:\\Windows\\System32\\log.txt to support"));
			Log(x("--------------------- map end failure ---------------------") << std::endl);
			return false;
		}

		Log(x("--------------------- map end success ---------------------") << std::endl);
		return true;
	}

	inline auto deinit()
	{
		if (pPort != nullptr)
		{
			pPort->bAlive = 0ull;
		}
	}

	inline auto queue(std::uint64_t buffer)
	{
		int test = 0;

		if (pPort->bAlive)
		{
			pPort->Buffer = buffer;
			pPort->bLock = 1ull;

			while (pPort->bLock)
			{
				test++;
				std::this_thread::sleep_for(std::chrono::nanoseconds(0));
			}
		}
	}

	inline auto TestConnection()
	{
		CommData::Request msg = {};
		msg.Type = CommData::RequestType::TestConnection;

		queue(reinterpret_cast<std::uint64_t>(&msg));
		return msg.Result == 0xDEAD2;
	}

	inline auto read(std::uint64_t address, std::uint64_t buffer, std::uint32_t size) -> std::size_t
	{
		CommData::Request msg = {};
		msg.Type = CommData::RequestType::ReadMemory;

		msg.ReadWriteMemory.ProcessID = processID;
		msg.ReadWriteMemory.Address = address;
		msg.ReadWriteMemory.Buffer = buffer;
		msg.ReadWriteMemory.Size = size;

		queue(reinterpret_cast<std::uint64_t>(&msg));
		return msg.Result;
	}
	template <class T> __forceinline T read(std::uint64_t address)
	{
		T buffer = {};
		read(
			address,
			reinterpret_cast<std::uint64_t>(&buffer),
			sizeof(T)
		);
		return buffer;
	}

	inline std::uint64_t write(std::uint64_t address, std::uint64_t buffer, std::uint32_t size)
	{
		CommData::Request msg = {};
		msg.Type = CommData::RequestType::WriteMemory;

		msg.ReadWriteMemory.ProcessID = processID;
		msg.ReadWriteMemory.Address = address;
		msg.ReadWriteMemory.Buffer = buffer;
		msg.ReadWriteMemory.Size = size;

		queue(reinterpret_cast<std::uint64_t>(&msg));
		return msg.Result;
	}
	template <class T> inline bool write(std::uint64_t address, const T& buffer)
	{
		return write(
			address,
			reinterpret_cast<std::uint64_t>(&buffer),
			sizeof(T)
		) == sizeof(T);
	}

	inline auto base() -> std::uint64_t
	{
		CommData::Request msg = {};
		msg.Type = CommData::RequestType::VirtualBase;

		msg.VirtualAddress.ProcessID = processID;

		queue(reinterpret_cast<std::uint64_t>(&msg));
		return msg.Result;
	}

	inline auto module(const wchar_t* mod_name) -> std::uint64_t
	{
		CommData::Request msg = {};
		msg.Type = CommData::RequestType::ModuleBase;

		msg.ModuleAddress.ProcessID = processID;
		wcscpy_s(msg.ModuleAddress.ModuleName, sizeof(msg.ModuleAddress.ModuleName) / sizeof(msg.ModuleAddress.ModuleName[0]), mod_name);

		queue(reinterpret_cast<std::uint64_t>(&msg));
		return msg.Result;
	}

	__forceinline std::uint64_t alloc(std::uint64_t address, std::uint64_t size, std::uint32_t type, std::uint32_t protect)
	{
		CommData::Request msg = {};
		msg.Type = CommData::RequestType::AllocateMemory;

		msg.AllocateProtect.ProcessID = processID;
		msg.AllocateProtect.Address = address;
		msg.AllocateProtect.Size = size;
		msg.AllocateProtect.Type = type;
		msg.AllocateProtect.Protection = protect;

		queue(reinterpret_cast<std::uint64_t>(&msg));
		return msg.Result;
	}

	__forceinline std::uint64_t protect(std::uint64_t address, std::uint32_t size, std::uint32_t protect)
	{
		CommData::Request msg = {};
		msg.Type = CommData::RequestType::ProtectMemory;

		msg.AllocateProtect.ProcessID = processID;
		msg.AllocateProtect.Address = address;
		msg.AllocateProtect.Size = size;
		msg.AllocateProtect.Protection = protect;

		queue(reinterpret_cast<std::uint64_t>(&msg));
		return msg.Result;
	}

	__forceinline auto attach()
	{
		Log(x("[>] Attempting to attach to the target") << std::endl);

		auto window = FindWindowA((auth::windowClass.length() == 0) ? 0 : auth::windowClass.c_str(), (auth::windowName.length() == 0) ? 0 : auth::windowName.c_str());
		if (!window)
		{
			Log(x("[>] Could not find Window") << std::endl);
			return false;
		}

		if (!GetWindowThreadProcessId(window, (LPDWORD)&processID))
		{
			Log(x("[>] Could not find TID") << std::endl);
			return false;
		}

		if (!processID)
		{
			Log(x("[>] Could not find PID") << std::endl);
			return false;
		}

		baseAddress = base();
		if (!baseAddress)
		{
			Log(x("[>] Could not find base") << std::endl);
			return false;
		}

		Log(x("[>] Attach success") << std::endl);
		return true;
	}
};