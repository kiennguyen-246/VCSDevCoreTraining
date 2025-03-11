// dllmain.cpp : Defines the entry point for the DLL application.
#include "Redirect.h"
#include "pch.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call,
                      LPVOID lpReserved) {
  switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
      WCHAR pwcPath[MAX_PATH];
      GetEnvironmentVariable(L"USERPROFILE", pwcPath, MAX_PATH);
      wcscat_s(pwcPath, MAX_PATH, L"\\Temp\\output.txt");
      FILE* pFile;
      _wfreopen_s(&pFile, pwcPath, L"w", stdout);

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
      break;
  }
  return TRUE;
}
