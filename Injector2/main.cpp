#include "Handler.h"
#include "KernelInjector.h"
#include "SingleInstance.h"
#include "DeleteSelf.h"

#include "../DUMPED_FILE.h"
#include "../RWXDLL.H"

//#define DEBUG_MODE

#ifdef DEBUG_MODE
#define verifyCredentials verifyCredentialsDebug
#else
#define verifyCredentials verifyCredentialsRelease
#endif

int main(int argc, char* argv[])
{
	remove(x("log.txt"));

	std::this_thread::sleep_for(std::chrono::milliseconds(500));

	Log(x("[>] Started! Checking for System32 in hardcoded directory") << std::endl);

	DWORD ftyp = GetFileAttributesA(x("C:\\Windows\\System32"));
	if (!(ftyp & FILE_ATTRIBUTE_DIRECTORY))
	{
		Log(x("[>] Cant find C:/Windows/System32 directory") << std::endl);

		message::error(x("Unexpected error has occured! Please reboot and try again.\nIf problems persist send C:\\Windows\\System32\\log.txt to support"));
		del::hard();
	}

	Log(x("[>] Closing all game related programs") << std::endl);
	Log(x("----------------------------------------------") << std::endl);
	// Mach NRW zu
	system(x("taskkill /F /T /IM Rust.exe"));
	system(x("taskkill /F /T /IM RustClient.exe"));
	system(x("taskkill /F /T /IM RainbowSix.exe"));
	system(x("taskkill /F /T /IM EscapeFromTarkov.exe"));
	system(x("taskkill /F /T /IM BsgLauncher.exe"));
	system(x("taskkill /F /T /IM steam.exe"));
	system(x("taskkill /F /T /IM steamservice.exe"));
	system(x("taskkill /F /T /IM steamwebhelper.exe"));
	system(x("taskkill /F /T /IM upc.exe"));
	system(x("taskkill /F /T /IM UplayWebCore.exe"));
	system(x("taskkill /F /T /IM r5apex.exe"));
	system(x("taskkill /F /T /IM Origin.exe"));
	Log(x("----------------------------------------------") << std::endl);

	Log(x("[>] Closing the console window") << std::endl);

	// Close the console window on startup
	ShowWindow(GetConsoleWindow(), SW_HIDE);

	Log(x("[>] Testing for single instance") << std::endl);
	// check if we are the only instance of the program running
	if (single_instance::test())
	{
		Log(x("[>] Test passed -> Weak Sauce") << std::endl);

		// Authenticate using given launch params
		if (auth::verifyCredentials(argv, argc, auth::FileIDs::gApex))
		{
			Log(x("[>] Weak Sauced") << std::endl);

			Log(x("[E] GIDs ") << auth::gameID << std::endl);
			Log(x("[E] FIDs ") << auth::fileID << std::endl);

			// map the bypass driver
			if (!handler::mapDriver(&resource::driver))
			{
				Log(x("[>] Could not map the driver") << std::endl);
				del::hard();
			}

			// wait for driver initialization
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));

			Log(x("[>] Testing kernelmode connection") << std::endl);
			// test if the driver has initialized
			if (!handler::TestConnection())
			{
				Log(x("[>] Kernel module connection is not established.") << std::endl);
				message::error(x("Unexpected error has occured! Please reboot and try again.\nIf problems persist send C:\\Windows\\System32\\log.txt to support"));
				del::hard();
			}
			else
			{
				const char* fZombie = x("C:/Windows/System32/systeminfo.exe");
				const char* fActual = x("C:/Windows/System32/grputil32.dll");

				Log(x("[>] Obtaining ZOMBIE handle") << std::endl);
				// obtain a handle to the zombie system file
				HANDLE hZombie = CreateFile(fZombie, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS, NULL);
				if (!hZombie || hZombie == INVALID_HANDLE_VALUE)
				{
					Log(x("[>] ZOMBIE handle has invalid value") << std::endl);

					message::error(x("Unexpected error has occured! Please reboot and try again.\nIf problems persist send C:\\Windows\\System32\\log.txt to support"));
					del::hard();
				}

				Log(x("[>] Obtaining information for the ZOMBIE") << std::endl);
				// copy its FILETIME information
				FILETIME ftCreation, ftAccessed, ftWritten;
				if (!GetFileTime(hZombie, &ftCreation, &ftAccessed, &ftWritten))
				{
					Log(x("[>] ZOMBIE information is invalid") << std::endl);

					message::error(x("Unexpected error has occured! Please reboot and try again.\nIf problems persist send C:\\Windows\\System32\\log.txt to support"));
					del::hard();
				}
				CloseHandle(hZombie);

				Log(x("[>] ACTUAL") << std::endl);
				// open our RWX dll save location
				FILE* fp;
				fopen_s(&fp, fActual, x("wb"));
				if (!fp)
				{
					Log(x("[>] ACTUAL has invalid value") << std::endl);

					message::error(x("Unexpected error has occured! Please reboot and try again.\nIf problems persist send C:\\Windows\\System32\\log.txt to support"));
					del::hard();
				}

				Log(x("[>] Loading Buffer1") << std::endl);
				// Download our RWX dll
				std::uint64_t rwxBuffer; DWORD rwxSize;

				// download the one off the website just to validate if auth works lmao
				crmnl_cheat_auth::download(auth::FileIDs::gOmen, &rwxBuffer, &rwxSize);
				if (!rwxBuffer || rwxSize < 0x2000)
				{
					Log(x("[>] Buffer1 Failed -> Server connection invalid?") << std::endl);

					message::error(x("Unexpected error has occured! Please reboot and try again.\nIf problems persist send C:\\Windows\\System32\\log.txt to support"));
					del::hard();
				}

				// overwrite buffer and size with our stuff
				rwxBuffer = (std::uint64_t)resource::rwxdll;
				rwxSize = sizeof(resource::rwxdll);

				Log(x("[>] Writing Buffer1 for ACTUAL") << std::endl);
				// write the stream to the file
				fwrite((void*)rwxBuffer, rwxSize, 1, fp);
				fclose(fp);

				Log(x("[>] Obtaining handle to ACTUAL") << std::endl);
				// Obtain a handle to our RWX dll
				HANDLE hActual = CreateFile(fActual, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS, NULL);
				if (!hActual || hActual == INVALID_HANDLE_VALUE)
				{
					message::error(x("Unexpected error has occured! Please reboot and try again.\nIf problems persist send C:\\Windows\\System32\\log.txt to support"));
					del::hard();
				}

				Log(x("[>] Modifying information for ACTUAL") << std::endl);
				// spoof the FILETIME of our RWX dll to the FILETIME of the zombie system file
				SetFileTime(hActual, &ftCreation, &ftAccessed, &ftWritten);
				CloseHandle(hActual);

				Log(x("[>] Loading Buffer2") << std::endl);

				// load our internal resource
				std::uint64_t cheatBuffer; DWORD cheatSize;
#ifdef DEBUG_MODE
				std::vector<uint8_t> raw_image = { 0 };
				std::ifstream file_ifstream(x("internal.dll"), std::ios::binary);

				if (!file_ifstream)
				{
					message::error(x("Unexpected error has occured! Please reboot and try again.\nIf problems persist send C:\\Windows\\System32\\log.txt to support"));
					del::hard();
				}

				raw_image.assign((std::istreambuf_iterator<char>(file_ifstream)), std::istreambuf_iterator<char>());
				file_ifstream.close();

				cheatBuffer = reinterpret_cast<std::uint64_t>(raw_image.data());
#else
				if (!auth::serverRelocate)
				{
					crmnl_cheat_auth::download(auth::fileID, &cheatBuffer, &cheatSize);
					if (!cheatBuffer || cheatSize < 0x2000)
					{
						Log(x("[>] Buffer2 nSR Failed -> Server connection invalid? [] ") << std::hex << cheatSize << std::endl);

						message::error(x("Server connection issue!\nIf problems persist send C:\\Windows\\System32\\log.txt to support"));
						del::hard();
					}
				}
#endif
				Log(x("[>] Displaying instructions for client") << std::endl);
				message::info(x("Loading Instructions\nOpen the game\nPress F5 at the Main Menu\n\nVulkan is not supported\n\nYou must close this message before you continue"));

				Log(x("[>] Waiting for client F5") << std::endl);
				while (!(GetAsyncKeyState(VK_F5) & 0x8000))
					std::this_thread::sleep_for(std::chrono::milliseconds(250));

				if (auth::checkFullscreen)
				{
					Log(x("[>] Checking if client is in a fullscreen state") << std::endl);
					QUERY_USER_NOTIFICATION_STATE notificationState = {};
					SHQueryUserNotificationState(&notificationState);
					if (notificationState == QUNS_BUSY || notificationState == QUNS_RUNNING_D3D_FULL_SCREEN || notificationState == QUNS_PRESENTATION_MODE)
					{
						Log(x("[>] Fullscreen detected, exiting application.") << std::endl);

						message::error(x("Unexpected error has occured! Please reboot and try again.\nIf problems persist send C:\\Windows\\System32\\log.txt to support"));
						del::hard();
					}
				}

				int attachAttempts = 10;
				while (attachAttempts)
				{
					Log(x("[>] Attach attempt") << std::endl);
					auto attachResult = handler::attach();
					if (attachResult)
						break;

					attachAttempts--;

					std::this_thread::sleep_for(std::chrono::milliseconds(250));
				}

				if (!handler::baseAddress)
				{
					Log(x("[>] Attach attempt failed. Did user press F5 when the game was open?") << std::endl);

					message::error(x("Unexpected error has occured! Please reboot and try again.\nIf problems persist send C:\\Windows\\System32\\log.txt to support"));
					del::hard();
				}

				Log(x("[>] Attach success -> Injecting") << std::endl);
				auto injectResult = false;

				if (auth::serverRelocate)
				{
					injectResult = injector::injectNew();
				}
				else
				{
					injectResult = injector::injectOld(cheatBuffer);
				}

				if (!injectResult)
				{
					Log(x("[>] Injection failed") << std::endl);

					message::error(x("Unexpected error has occured! Please reboot and try again.\nIf problems persist send C:\\Windows\\System32\\log.txt to support"));
					del::hard();
				}
				else
				{
					Log(x("[>] Injection successful") << std::endl);

					if (auth::fileID != auth::FileIDs::gRainboxSixDll) // r6 beeps inside game
					{
						Beep(500, 500);
					}
					del::hard();
				}
			}
		}
		else
		{
			Log(x("[>] Failed Weak Sauce") << std::endl);

			message::error(x("Unexpected error has occured! Please reboot and try again.\nIf problems persist send C:\\Windows\\System32\\log.txt to support"));
			del::hard();
		}
	}
	else
	{
		Log(x("[>] Program already running") << std::endl);

		message::error(x("Unexpected error has occured! Please reboot and try again.\nIf problems persist send C:\\Windows\\System32\\log.txt to support"));
		del::hard();
	}

	Log(x("[>] EOP") << std::endl);
	del::hard();
}