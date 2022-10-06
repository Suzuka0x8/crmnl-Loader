#pragma once

namespace CommData
{
	enum RequestType : int
	{
		Unload = 1,
		TestConnection,
		ReadMemory,
		WriteMemory,
		VirtualBase,
		ModuleBase,
		AllocateMemory,
		ProtectMemory
	};

	struct DriverEntry
	{
		unsigned __int64	pPort;
		unsigned __int32	ProcessID;
	};

	struct Request
	{
		int					Type;
		unsigned __int64	Result;

		union
		{
			struct
			{
				unsigned __int32	ProcessID;
				unsigned __int64	Address;
				unsigned __int64	Buffer;
				unsigned __int32	Size;
			} ReadWriteMemory;

			struct
			{
				unsigned __int32	ProcessID;
			} VirtualAddress;

			struct
			{
				unsigned __int32	ProcessID;
				wchar_t				ModuleName[255];
			} ModuleAddress;

			struct
			{
				unsigned __int32	ProcessID;
				unsigned __int64	Size;
				unsigned __int64	Address;
				unsigned __int32	Type;
				unsigned __int32	Protection;
			} AllocateProtect;
		};
	};

	struct Port
	{
		unsigned __int64	MagicValue;
		unsigned __int64	bLock;
		unsigned __int64	bAlive;
		unsigned __int64	Buffer;
	};
}