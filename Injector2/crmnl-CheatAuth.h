#pragma once


#include <cstdint>
#include <string>

#include "XorString.h"

namespace crmnl_cheat_auth
{
	bool verify_user(BYTE gameid, const char* username, const char* password);
	void download(BYTE file_id, uintptr_t* buffer, DWORD* size);
	void download_relocated(BYTE file_id, uintptr_t* buffer, DWORD* size, uintptr_t relocatedbase);
}

namespace auth
{
	inline std::uint8_t gameID;
	inline std::string username;
	inline std::string password;

	inline std::uint8_t fileID;
	inline std::string windowClass;
	inline std::string windowName;

	inline bool serverRelocate;
	inline bool debugMode;
	inline bool checkFullscreen;

	enum FileIDs
	{
		gRainbowSix,
		gEFT,
		gApex,
		gCsgo,
		gSpoofer,
		gRust,
		gSplitgate,
		gModernWarfare,
		gChess,
		gFortnite,
		gHalo,
		gRainboxSixDll,
		gEFTDll,
		gloader = 100,
		gHelloCNet = 101,
		gHelloCpp = 102,
		gApexDebug = 103,
		gRwxDriver = 104,
		gRwx64 = 105,
		gRwx32 = 106,
		gEFTDebug = 107,
		gOmen = 200,
		gNth = 201,
		gNth2 = 202,
		api1 = 203,
		api2 = 204,
	};

	inline auto set_authGlobals()
	{
		debugMode = std::filesystem::exists(x("C:\\debug.txt"));

		switch (gameID)
		{
		case FileIDs::gRainbowSix:
		{
			windowClass = x("R6Game");
			windowName = x("");
			fileID = FileIDs::gRainboxSixDll;

			serverRelocate = true;
			checkFullscreen = false;

			break;
		}
		case FileIDs::gEFT
		{
			windowClass = x("");
			windowName = x("EscapeFromTarkov");
			fileID = FileIDs::gEFTDll;

			serverRelocate = false;
			checkFullscreen = false;

			break;
		}
		case FileIDs::gRust:
		{
			gameID = 0; // temp

			windowClass = x("UnityWndClass");
			windowName = x("Rust");
			fileID = -1; // temp

			serverRelocate = false;
			checkFullscreen = false;

			break;
		}
		case FileIDs::gApex:
		{
			windowClass = x("Respawn001");
			windowName = x("Apex Legends");
			fileID = debugMode ? FileIDs::gApexDebug : FileIDs::gApex;

			serverRelocate = false;
			checkFullscreen = false;

			break;
		}
		default:
			break;
		}
	}

	static inline auto verifyCredentialsRelease(char* Buffer[], int size, int reserved) -> bool
	{
		if (size == 3)
		{
			gameID = std::atoi(Buffer[0]);
			username = std::string(Buffer[1]);
			password = std::string(Buffer[2]);

			set_authGlobals();

			return crmnl_cheat_auth::verify_user(gameID, username.c_str(), password.c_str());
		}
		return false;
	}

	static inline auto verifyCredentialsDebug(char* reservedbuff[], int rservedint, int GameID) -> bool
	{
		gameID = GameID;
		username = x("Suzuka0x8"); // Suzuka
		password = x("Qaymlp10"); //1030379 Oaymlp10

		set_authGlobals();

		return crmnl_cheat_auth::verify_user(gameID, username.c_str(), password.c_str());
	}
}
