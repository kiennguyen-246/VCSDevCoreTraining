// Test.cpp : This file contains the 'main' function. Program execution begins
// and ends there.
//

#include <Windows.h>

#include <iostream>
#include <string>

#include "Notifications.h"

int localNotify(wchar_t* pwcMsg) {
  return MessageBox(NULL, pwcMsg, L"Message Box from local process",
                    MB_OK | MB_ICONASTERISK);
}

int main() {
  for (int i = 1; i <= 100; i++) {
    Sleep(10000);
    std::wstring wstr = std::to_wstring(10 * i) + L" seconds has passed";
    notify(&wstr[0]);
  }
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started:
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add
//   Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project
//   and select the .sln file
