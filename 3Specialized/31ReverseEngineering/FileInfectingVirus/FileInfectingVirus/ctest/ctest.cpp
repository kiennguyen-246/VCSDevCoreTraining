// ctest.cpp : This file contains the 'main' function. Program execution begins
// and ends there.
//

#include <Windows.h>
#include <winternl.h>

#include <iostream>

int main() {
  char x[260];
   x[0] = 'u';
   x[1] = 's';
   x[2] = 'e';
   x[3] = 'r';
   x[4] = '3';
   x[5] = '2';
   x[6] = '.';
   x[7] = 'd';
   x[8] = 'l';
   x[9] = 'l';
   x[10] = 0;
   auto t = LoadLibraryA(x);

   /*HMODULE hmod = GetModuleHandleA(NULL);
   PCHAR pcBuffer = (PCHAR)VirtualAlloc(NULL, 10, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
   WriteProcessMemory(hmod, pcBuffer, x, 10, NULL);
   t = GetLastError();
   t = t;*/

   //auto u = GetProcAddress(t, "LoadLibraryA");
   //auto v = (PVOID*)u - (PVOID*)t;
   //v = v;
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
