// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call,
                      LPVOID lpReserved) {
  MessageBox(NULL,
             TEXT("This message box is displayed by MessageBox.dll, which was ")
                 TEXT("injected into your process"),
             TEXT("MessageBox.dll is injected"), MB_OK | MB_ICONASTERISK);
  switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:

      break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
      break;
  }
  return TRUE;
}
