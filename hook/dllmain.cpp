// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <stdio.h>
//#define DEBUG

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

#ifdef DOUBLECLICK_MOD
LARGE_INTEGER lastClickTime = { 0 };
LARGE_INTEGER frequency = { 0 };
#endif

LRESULT CALLBACK hookProc(int code, WPARAM wParam, LPARAM lParam) {
	CWPSTRUCT* message = (CWPSTRUCT*)lParam;
	dprintf("thing (%d %u %ld %u)\n", code, wParam, lParam, message->message);

	if (memcmp("VT", (void*)0x004DCBE4, 2) == 0) {
		goto end;
	}

	if (code < 0) {
		return CallNextHookEx(NULL, code, wParam, lParam);
	}

	switch (message->message) {
	case WM_MOUSEWHEEL: {
		short zDelta = GET_WHEEL_DELTA_WPARAM(message->wParam);
		dprintf("wheel (%d) (%X)\n", zDelta, message->wParam);
		if (zDelta > 0) {
			dprintf("scrolled up (%d)\n", zDelta);
			INPUT inputs[2];
			inputs[0].type = INPUT_KEYBOARD;
			inputs[0].ki.wVk = VK_F7;
			inputs[1].type = INPUT_KEYBOARD;
			inputs[1].ki.wVk = VK_F7;
			inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
			SendInput(2, inputs, sizeof(INPUT));
		}
		else if (zDelta < 0) {
			dprintf("scrolled down (%d)\n", zDelta);
			INPUT inputs[2];
			inputs[0].type = INPUT_KEYBOARD;
			inputs[0].ki.wVk = VK_F8;
			inputs[1].type = INPUT_KEYBOARD;
			inputs[1].ki.wVk = VK_F8;
			inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
			SendInput(2, inputs, sizeof(INPUT));
		}
	} break;
#ifdef DOUBLECLICK_MOD
	case WM_PARENTNOTIFY: {
		if (LOWORD(message->wParam) == WM_LBUTTONDOWN) {
			if (lastClickTime.QuadPart == 0) {
				QueryPerformanceCounter(&lastClickTime);
				break;
			}
			else {
				LARGE_INTEGER currentTime;
				double diff;
				QueryPerformanceCounter(&currentTime);
				diff = (currentTime.QuadPart - lastClickTime.QuadPart) / (double)frequency.QuadPart;
				if ((diff * 1000.0) < (double)GetDoubleClickTime()) {
					INPUT inputs[2];
					inputs[0].type = INPUT_KEYBOARD;
					inputs[0].ki.wVk = VK_RETURN;
					inputs[1].type = INPUT_KEYBOARD;
					inputs[1].ki.wVk = VK_RETURN;
					inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
					SendInput(2, inputs, sizeof(INPUT));
				}
				QueryPerformanceCounter(&lastClickTime);
			}
		}
	} break;
#endif
	}
end:
	return CallNextHookEx(NULL, code, wParam, lParam);
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
#ifdef DOUBLECLICK_MOD
		lastClickTime.QuadPart = 0;
		QueryPerformanceFrequency(&frequency);
#endif
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

