#define _CRT_SECURE_NO_WARNINGS
#include "pch.h"
#include <stdio.h>

#ifdef DEBUG
#define dprintf(fmt, ...) printf(fmt, __VA_ARGS__)
#else
#define dprintf(fmt, ...)
#endif

#ifdef DEBUG
#define dfprintf(file, fmt, ...) fprintf(file, fmt, __VA_ARGS__)
#else
#define dfprintf(file, fmt, ...)
#endif

#ifdef DEBUG
#define dfclose(fp) fclose(fp)
#else
#define dfclose(fp)
#endif

typedef BOOL(WINAPI* EPMFunction)(HANDLE hProcess, HMODULE* lphModule, DWORD cb, LPDWORD lpcbNeeded);
typedef DWORD(WINAPI* GMBNAFunction)(HANDLE hProcess, HMODULE hModules, LPSTR lpBaseName, DWORD nSize);
typedef BOOL(WINAPI* EPFunction)(DWORD* lpidProcess, DWORD cb, LPDWORD lpcbNeeded);

EPMFunction origEPM = NULL;
GMBNAFunction origGMBNA = NULL;
EPFunction origEP = NULL;
HMODULE self = NULL;
HMODULE hooklib = NULL;
HOOKPROC hookProc = NULL;
HHOOK hHook;
DWORD mainThread;
FILE* logFile;

BOOL WINAPI hEnumProcessModules(HANDLE hProcess, HMODULE* lphModule, DWORD cb, LPDWORD lpcbNeeded) {
	return origEPM(hProcess, lphModule, cb, lpcbNeeded);
}
DWORD WINAPI hGetModuleBaseNameA(HANDLE hProcess, HMODULE hModules, LPSTR lpBaseName, DWORD nSize) {
	return origGMBNA(hProcess, hModules, lpBaseName, nSize);
}
BOOL WINAPI hEnumProcesses(DWORD* lpidProcess, DWORD cb, LPDWORD lpcbNeeded) {
	return origEP(lpidProcess, cb, lpcbNeeded);
}

void setUpProcs(void) {
	HMODULE lib = LoadLibraryExA("psapiorig.dll", NULL, LOAD_LIBRARY_SEARCH_APPLICATION_DIR);
	hooklib = LoadLibraryExA("hook.dll", NULL, LOAD_LIBRARY_SEARCH_APPLICATION_DIR);
	hookProc = GetProcAddress(hooklib, "hookProc");

	origEPM = (EPMFunction)GetProcAddress(lib, "EnumProcessModules");;
	origGMBNA = (GMBNAFunction)GetProcAddress(lib, "GetModuleBaseNameA");
	origEP = (EPFunction)GetProcAddress(lib, "EnumProcesses");
}

BOOL CALLBACK enumProc(HWND hWnd, LPARAM lParam) {
	DWORD wndPid = 0;
	char className[20] = { 0 };
	GetWindowThreadProcessId(hWnd, &wndPid);
	GetClassNameA(hWnd, className, 20);
	if (wndPid == GetCurrentProcessId() && (strcmp(className, "PCSWS:Main:00400000") == 0)) {
		*((DWORD*)lParam) = GetWindowThreadProcessId(hWnd, NULL);
		return FALSE;
	}
	return TRUE;
}

DWORD WINAPI mouseHandler(LPVOID param) {
	DWORD wndTid = 0;
	dprintf("mouse handler up\n");
	while (!wndTid) {
		EnumWindows(enumProc, &wndTid);
	}
	dprintf("wndTid: %d\n", wndTid);
	hHook = SetWindowsHookExA(WH_CALLWNDPROC, hookProc, hooklib, wndTid);
	dprintf("hHook: %p\n", hHook);
	if (!hHook) {
		char* messageBuffer;
		size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
		dprintf("hook failed with error: %s\n", messageBuffer);
		dfprintf(logFile, "hook failed with error: %s\n", messageBuffer);
		dfclose(logFile);
		LocalFree(messageBuffer);
		TerminateProcess(GetCurrentProcessId(), 5);
	}

	HANDLE hThread = OpenThread(SYNCHRONIZE, FALSE, wndTid);
	if (!hThread) {
		char* messageBuffer;
		size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
		dprintf("openthread failed with error: %s\n", messageBuffer);
		dfprintf(logFile, "openthreadfailed with error: %s\n", messageBuffer);
		dfclose(logFile);
		LocalFree(messageBuffer);
		TerminateProcess(GetCurrentProcessId(), 6);
	}

	WaitForSingleObject(hThread, INFINITE);
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
#ifdef DEBUG
		AllocConsole();
		FILE* fp;
		freopen_s(&fp, "CONOUT$", "w", stdout);
#endif
		setUpProcs();
		self = hModule;
		mainThread = GetCurrentThreadId();
#ifdef DEBUG
		logFile = fopen("C:\\Users\\kohuept\\Downloads\\psapilog.txt", "w");
#endif
		DWORD tid;
		CreateThread(NULL, 1000000, mouseHandler, NULL, 0, &tid);
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		dfclose(logFile);
		UnhookWindowsHookEx(hHook);
		break;
	}
	return TRUE;
}

