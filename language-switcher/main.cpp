#include <stdbool.h>
#include <windows.h>
#include <tchar.h>

#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

UINT toggle_key = VK_CAPITAL;
HHOOK handle_hook;
HANDLE handle_event;
UINT_PTR timer_id;

bool is_key_hold = false, is_key_execute = false;
bool is_caplock = false;
ULONGLONG key_start;

static void SendKey(WORD vk_code, bool is_down) {
	INPUT input = { 0 };
	input.type = INPUT_KEYBOARD;
	input.ki.wVk = vk_code;
	input.ki.dwFlags = is_down ? 0 : KEYEVENTF_KEYUP;

	if (vk_code == VK_LWIN) {
		input.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
	}

	SendInput(1, &input, sizeof(INPUT));
}

static LRESULT CALLBACK keyboard_hook(int n_code, WPARAM w_param, LPARAM l_param) {
	if (n_code == HC_ACTION) {
		KBDLLHOOKSTRUCT* s_keyboard_hook = (KBDLLHOOKSTRUCT*)l_param;

		if (s_keyboard_hook->vkCode == toggle_key && !is_caplock) {

			if (w_param == WM_KEYDOWN || w_param == WM_SYSKEYDOWN) {
				if (!is_key_hold) {
					is_key_hold = true;
					key_start = GetTickCount64();
				}
				else if (!is_key_execute) {
					if ((GetTickCount64() - key_start) >= 1000) {
						is_key_execute = true;
						is_caplock = true;

						SendKey(VK_CAPITAL, true);
						SendKey(VK_CAPITAL, false);

						is_caplock = false;
					}
				}
				return 1;
			}

			if (w_param == WM_KEYUP || w_param == WM_SYSKEYUP) {
				if (is_key_hold && !is_key_execute) {
					if ((GetTickCount64() - key_start) < 1000) {
						SendKey(VK_LWIN, true);
						SendKey(VK_SPACE, true);
						SendKey(VK_SPACE, false);
						SendKey(VK_LWIN, false);
					}
				}
				is_key_hold = false;
				is_key_execute = false;

				return 1;
			}
		}
	}
	return CallNextHookEx(handle_hook, n_code, w_param, l_param);
}

static void CALLBACK timer_callback(HWND handle_window, UINT u_message, UINT_PTR event_id, DWORD dw_time) {
	if (WaitForSingleObject(handle_event, 0) == WAIT_OBJECT_0) {
		PostQuitMessage(0);
	}
}

int main() {
	handle_event = CreateEvent(NULL, true, false, _T("Language Switcher"));
	if (handle_event == NULL) {
		MessageBox(NULL, _T("Unable to start the program."), _T("Error"), MB_OK | MB_ICONERROR);
		
		return 1;
	}

	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		SetEvent(handle_event);
		CloseHandle(handle_event);
		MessageBox(NULL, _T("Program is ended."), _T("Success"), MB_OK | MB_ICONINFORMATION);
		
		return 0;
	}

	timer_id = SetTimer(NULL, 0, 1000, timer_callback);
	if (timer_id == 0) {
		MessageBox(NULL, _T("Unable to create timer."), _T("Error"), MB_OK | MB_ICONERROR);
		
		return 1;
	}

	handle_hook = SetWindowsHookEx(WH_KEYBOARD_LL, keyboard_hook, GetModuleHandle(NULL), 0);
	if (handle_hook == NULL) {
		MessageBox(NULL, _T("Unable to install hook."), _T("Error"), MB_OK | MB_ICONERROR);
		
		return 1;
	}

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	UnhookWindowsHookEx(handle_hook);
	KillTimer(NULL, timer_id);
	CloseHandle(handle_event);

	return 0;
}