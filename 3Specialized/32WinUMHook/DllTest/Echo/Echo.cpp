#include <windows.h>

#include <iostream>

int wmain(DWORD argc, LPWSTR argv[]) {
  int iErr = 0;

  //MessageBox(NULL,
  //           TEXT("This message box is displayed by MessageBox.dll, which was ")
  //               TEXT("injected into your process"),
  //           TEXT("MessageBox.dll is injected"), MB_OK | MB_ICONASTERISK);

   HINSTANCE hLib = LoadLibrary(L"MessageBox.dll");
   if (hLib == NULL) {
     iErr = GetLastError();
     std::cout << "LoadLibrary failed with error " << iErr << std::endl;
     return iErr;
   }

  // typedef void (*FpRedirect)(const WCHAR[]);
  // FpRedirect fpRedirect = (FpRedirect)GetProcAddress(hLib, "redirect");
  // if (fpRedirect == NULL) {
  //   iErr = GetLastError();
  //   std::cout << "GetProcAddress failed with error " << iErr << std::endl;
  //   FreeLibrary(hLib);
  //   return iErr;
  // }
  //(fpRedirect)(L"output.txt");

  if (argc <= 1) {
    std::wcout << 0;
  } else {
    std::wcout << argv[1];
  }

  FreeLibrary(hLib);
}