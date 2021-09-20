#include <iostream>
#include <Windows.h>

#pragma warning(disable : 4067 4996)

template<int PathSize>
struct Args
{
	char args[PathSize] = {};

	void ClearOut()
	{
		memset(args, 0, PathSize);
	}

	void Parse(int argc, const char* argv[])
	{
		ClearOut();
		if (argc > 1)
		{
			int cursor = 0;
			for (int i = 1; i < argc; i++)
			{
				if (cursor > PathSize - 1)
				{
					MessageBoxA(0, "Oh.\nArg Count is too big.\nWell thats sad...", "Launcher Error", 0);
					break;
				}
				strcpy(args + cursor, argv[i]);
				args[cursor + strlen(argv[i])] = ' ';
				cursor += strlen(argv[i]) + 1;
			}
		}
	}
}; 

inline Args<1024 * 4> args;
inline HMODULE launcherModule = 0;

void LaunchIt(HMODULE toLaunchFrom, int p2, char* commandLine, int flags)
{
	const char* EnvVar_Path = getenv("PATH");
	char moduleLaunchPath[0x105] = { 0 };
	char currentPath[0x500 + 1] = { 0 };
	char pathBinEnvText[0x1000] = {};
	GetCurrentDirectoryA(0x500, currentPath);

	sprintf_s(pathBinEnvText, "PATH=%s\\bin\\;%s", currentPath, EnvVar_Path);
	_putenv(pathBinEnvText);

	memset(pathBinEnvText, 0, sizeof(pathBinEnvText));
	sprintf_s(pathBinEnvText, "%s\\bin\\launcher.dll", currentPath);

	DWORD res = GetModuleFileNameA(toLaunchFrom, moduleLaunchPath, 0x104);

	if (res == 0)
	{
		MessageBoxA((HWND)0x0, "Failed calling GetModuleFileName", "Launcher Error", 0);
		return;
	}
	
	launcherModule = LoadLibraryExA(pathBinEnvText, 0, 8);
	if (launcherModule == 0)
	{
		char errorBuffer[0x100] = {};
		char errorBufferFormatted[0x150] = {};
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			errorBuffer, (sizeof(errorBuffer) / sizeof(char)), (va_list*)toLaunchFrom);

		sprintf_s(errorBufferFormatted, "Failed to load the launcher DLL:\n\n%s", errorBuffer);

		MessageBoxA((HWND)0x0, errorBufferFormatted, "Launcher Error", 0);

		return;
	}

	using mainLaunch = void(*)(HMODULE launchMod, int p2,char* cmdLine, int flags);
	mainLaunch fmainLaunch;
	fmainLaunch = (mainLaunch) GetProcAddress(launcherModule, "LauncherMain");
	printf("Bye for now! \n");
	printf("Have a good time for now! Please forget all the world's harm and people and just play your mod! \n");
	fmainLaunch(toLaunchFrom, p2, args.args, flags);
}

int main(int argc, const char* argv[])
{
	args.ClearOut();
	args.Parse(argc, argv);

	OSVERSIONINFOA* sysInfo = (OSVERSIONINFOA*) calloc(1, sizeof(OSVERSIONINFOA));

	// MainLauncher.txt (DECOMPILED swarm.exe) : Line 36
	sysInfo->dwOSVersionInfoSize = 0x94;
	auto buildNum = sysInfo->dwBuildNumber & 0x7fff;

	GetVersionExA(sysInfo);

	if (sysInfo->dwPlatformId != 2) {
		buildNum = buildNum | 0x8000;
	}

	printf("Launching...\n");
	LaunchIt(GetModuleHandleA(0), 0, args.args, buildNum);

	free(sysInfo);
}
