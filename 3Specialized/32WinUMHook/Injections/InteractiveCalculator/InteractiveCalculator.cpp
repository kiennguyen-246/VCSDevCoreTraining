#include <windows.h>

#include <iostream>

int wmain() {
  std::wcout << "Interactive Calculator\n";
  std::wcout << "1. Simple Adder Calculator\n";
  std::wcout << "2. Age Calculator\n";
  std::wcout << "3. Spam\n";
  std::wcout << "0. Quit\n";

  int choice;
  std::wcin >> choice;

  int a, b;

  switch (choice) {
    case 1:
      std::wcout << "Type a\n";
      std::wcin >> a;
      std::wcout << "Type b\n";
      std::wcin >> b;
      std::wcout << "a + b = " << a + b << "\n ";
      break;
    case 2:
      std::wcout << "Type your age\n";
      std::wcin >> a;
      std::wcout << "Your age is " << a << ".\n";
      break;
    case 3:
      for (int i = 1; i <= 10000; i++) {
        std::wcout << i << "\n";
        Sleep(1000);
      }
      break;
    default:
      break;
  }

  return 0;
}