// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

typedef int (*FP_NOTIFY)(wchar_t*);

HANDLE hPipe;
FP_NOTIFY fpNotify;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call,
                      LPVOID lpReserved) {
  // hHookedLibrary = GetModuleHandle(L"Notification.dll");
  hPipe = CreateFile(L"\\\\.\\pipe\\HookPipe", GENERIC_READ | GENERIC_WRITE, 0,
                     NULL, OPEN_EXISTING, 0, NULL);
  switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
      break;
  }
  return TRUE;
}

typedef struct _GET_FUNCTION_ADDRESS_INPUT {
  char pcDllName[MAX_PATH];
  char pcFuncName[MAX_PATH];
} GET_FUNCTION_ADDRESS_INPUT, *PGET_FUNCTION_ADDRESS_INPUT;

extern "C" {
/* Hook function */
__declspec(dllexport) int fakeNotify(wchar_t* pwcMsg) {
  std::wstring wsNewMsg =
      L"I caught your message (￣y▽￣)╭ \r\n\"" + std::wstring(pwcMsg) + L"\"";
  MessageBox(NULL, wsNewMsg.c_str(), L"Process hooked by FakeNotification.dll",
             MB_OK | MB_ICONERROR);
  BYTE abBuffer[64];
  DWORD dwBytesRead = 0;
  ZeroMemory(abBuffer, 64);
  if (!fpNotify) {
    ReadFile(hPipe, abBuffer, 64, &dwBytesRead, NULL);
    ULONGLONG ullFunctionPoint = 0;
    for (int i = 0; i < 64; i++)
      ullFunctionPoint = ullFunctionPoint * 2 + (abBuffer[i] - '0');
    fpNotify = (FP_NOTIFY)(ullFunctionPoint);
    MessageBox(NULL,
               std::format(L"Function pointer: 0x{:16x}", ullFunctionPoint).c_str(),
               L"fakeNotify", MB_OK | MB_ICONASTERISK);
  }

  return (fpNotify)(pwcMsg);
}

/* Get the target image base */
__declspec(dllexport) void findLoadProcessImageBase() {
  HMODULE hLoadProcessImageBase = GetModuleHandle(NULL);
  if (!hLoadProcessImageBase) {
    MessageBox(NULL, L"Error getting module", L"findLoadProcessImageBase",
               MB_OK | MB_ICONWARNING);
  }
  CHAR abBuffer[64];
  ZeroMemory(abBuffer, 64);
  for (int i = 0; i < 63; i++) {
    abBuffer[63 - i] = (((ULONGLONG)(hLoadProcessImageBase) >> i) & 1) + '0';
  }
  DWORD dwBytesWritten = 0;
  WriteFile(hPipe, abBuffer, 64, &dwBytesWritten, NULL);
}

/* Get the load base of some modules in the target process */
__declspec(dllexport) void getModuleAddress(
    char* pcModuleName) {
  HMODULE hLib = GetModuleHandleA(pcModuleName);
  if (!hLib) {
    DWORD dwErr = GetLastError();
    MessageBox(NULL, std::format(L"GetModuleHandle failed {}", dwErr).c_str(),
               L"getModuleAddress", MB_OK | MB_ICONSTOP);
  }
  //MessageBox(NULL,
  //           std::format(L"{} {}", (ULONGLONG)hLib, (ULONGLONG)fpFunc).c_str(),
  //           L"getModuleAddress", MB_OK | MB_ICONASTERISK);
  CHAR abBuffer[64];
  ZeroMemory(abBuffer, 64);
  for (int i = 0; i < 63; i++) {
    abBuffer[63 - i] = (((ULONGLONG)(hLib) >> i) & 1) + '0';
  }
  DWORD dwBytesWritten = 0;
  if (!WriteFile(hPipe, abBuffer, 64, &dwBytesWritten, NULL)) {
    DWORD dwErr = GetLastError();
    MessageBox(NULL, std::format(L"WriteFile failed {}", dwErr).c_str(),
               L"getModuleAddress", MB_OK | MB_ICONSTOP);
  }
}
}