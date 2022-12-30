#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>
#include <tchar.h>

UINT	toggle_key = VK_CAPITAL;
HHOOK	handle_hook;
HANDLE  handle_event;

LRESULT CALLBACK keyboard_hook(int n_code, WPARAM w_param,LPARAM l_param) {
	if (n_code == HC_ACTION) {
		KBDLLHOOKSTRUCT* s_keyboard_hook = (KBDLLHOOKSTRUCT*) l_param;
		HWND handle_window = GetForegroundWindow();
		if (s_keyboard_hook->vkCode == toggle_key) {
			if (w_param == WM_KEYDOWN && handle_window != NULL) {
				PostMessage(handle_window, WM_INPUTLANGCHANGEREQUEST, 0, HKL_NEXT);
			}
			return 1;
		}
	}
	return CallNextHookEx(handle_hook, n_code, w_param, l_param);
}

void CALLBACK timer_callback(HWND handle_window, UINT u_message, UINT_PTR event_id, DWORD dw_time) {
	if (WaitForSingleObject(handle_event, 0) == WAIT_OBJECT_0) {
		PostQuitMessage(0);
	}
}

int main(int argc, char** argv) {
	bool is_quit = false;
	
	if (argc > 2) {
		printf("ERROR: invalid number of arguments.");
	}
	if (argc == 1) {
		is_quit = true;
	}
	if (argc == 2) {
		toggle_key = atoi(argv[1]);
	}

	handle_event = CreateEvent(NULL, true, false, _T("Language Switcher"));
	if (handle_event == NULL) {
		printf("ERROR: unable to start a program.");
		return 1;
	}
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		if (is_quit) {
			SetEvent(handle_event);
			CloseHandle(handle_event);
			printf("SUCCESS: program is ended.");
			return 0;
		}
		else {
			printf("ERROR: program is already running...");
			return 1;
		}
	}
	
	if (SetTimer(NULL, 0, 1000, timer_callback) == NULL) {
		printf("ERROR: unable to start a program.");
		return 1;
	}

	handle_hook = SetWindowsHookEx(WH_KEYBOARD_LL, keyboard_hook, GetModuleHandle(NULL), 0);
	if (handle_hook == NULL) {
		printf("ERROR: unable to start a program.");
		return 1;
	}

	MSG msg;
	while (GetMessage(&msg,0,0,0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}