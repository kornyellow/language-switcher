#include <stdbool.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include <time.h>

UINT	toggle_key = VK_CAPITAL;
HHOOK	handle_hook;
HANDLE	handle_event;

bool	is_key_hold = false, is_key_execute = false;
bool	is_caplock = false;
clock_t	key_start, key_end;
double	key_total;

LRESULT CALLBACK keyboard_hook(int n_code, WPARAM w_param, LPARAM l_param) {
	if (n_code == HC_ACTION) {
		KBDLLHOOKSTRUCT* s_keyboard_hook = (KBDLLHOOKSTRUCT*) l_param;
		HWND handle_window = GetForegroundWindow();
		if (s_keyboard_hook->vkCode == toggle_key && !is_caplock) {
			if (w_param == WM_KEYDOWN && handle_window != NULL && !is_key_hold) {
				is_key_hold = true;
				key_start = clock();
			}
			if (w_param == WM_KEYDOWN && handle_window != NULL && is_key_hold && !is_key_execute) {
				key_end = clock();
				key_total = (double)(key_end - key_start) / CLOCKS_PER_SEC;

				if (key_total >= 1) {
					is_key_execute = true;

					is_caplock = true;
					keybd_event(VK_CAPITAL, 0x45, KEYEVENTF_EXTENDEDKEY|0, 0);
					keybd_event(VK_CAPITAL, 0x45, KEYEVENTF_EXTENDEDKEY|KEYEVENTF_KEYUP, 0);
					is_caplock = false;
				}
			}
			if (w_param == WM_KEYUP && handle_window != NULL) {
				key_end = clock();
				key_total = (double)(key_end - key_start) / CLOCKS_PER_SEC;
				
				is_key_hold = false;
				is_key_execute = false;
				is_caplock = false;
				if (key_total < 1) {
					PostMessage(handle_window, WM_INPUTLANGCHANGEREQUEST, 0, HKL_NEXT);
				}
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
	ShowWindow(FindWindowA("ConsoleWindowClass", NULL), false);
	FreeConsole();

	bool is_quit = false;
	
	if (argc > 2) {
		MessageBox(NULL, _T("Invalid number of arguments"), _T("Error"), MB_OK|MB_ICONERROR);
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
		MessageBox(NULL, _T("Unable to start a program"), _T("Error"), MB_OK|MB_ICONERROR);
		printf("ERROR: unable to start a program.");
		return 1;
	}
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		if (is_quit) {
			SetEvent(handle_event);
			CloseHandle(handle_event);
			MessageBox(NULL, _T("Program is ended"), _T("Success"), MB_OK|MB_ICONINFORMATION);
			printf("SUCCESS: program is ended.");
			return 0;
		}
		else {
			MessageBox(NULL, _T("Program is already running..."), _T("Error"), MB_OK|MB_ICONERROR);
			printf("ERROR: program is already running...");
			return 1;
		}
	}
	
	if (SetTimer(NULL, 0, 1000, timer_callback) == NULL) {
		MessageBox(NULL, _T("Unable to start a program"), _T("Error"), MB_OK|MB_ICONERROR);
		printf("ERROR: unable to start a program.");
		return 1;
	}

	handle_hook = SetWindowsHookEx(WH_KEYBOARD_LL, keyboard_hook, GetModuleHandle(NULL), 0);
	if (handle_hook == NULL) {
		MessageBox(NULL, _T("Unable to start a program"), _T("Error"), MB_OK|MB_ICONERROR);
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