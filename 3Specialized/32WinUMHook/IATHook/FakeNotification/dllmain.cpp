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

extern "C" {
__declspec(dllexport) int fakeNotify(wchar_t* pwcMsg) {
  std::wstring wsNewMsg =
      L"I caught your message (￣y▽￣)╭ \r\n\"" + std::wstring(pwcMsg) + L"\"";
  MessageBox(NULL, wsNewMsg.c_str(), L"Process hooked by FakeNotification.dll",
             MB_OK | MB_ICONERROR);
  // HANDLE hFile =
  //     CreateFile(L"C:\\Users\\a\\Temp\\x", GENERIC_READ | GENERIC_WRITE,
  //                FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS,
  //                FILE_ATTRIBUTE_NORMAL, NULL);
  BYTE abBuffer[64];
  DWORD dwBytesRead = 0;
  ZeroMemory(abBuffer, 64);
  ReadFile(hPipe, abBuffer, 64, &dwBytesRead, NULL);
  // CloseHandle(hFile);
  ULONGLONG ullFunctionPoint = 0;
  for (int i = 0; i < 64; i++)
    ullFunctionPoint = ullFunctionPoint * 2 + (abBuffer[i] - '0');

  if (!fpNotify) {
    fpNotify = (FP_NOTIFY)(ullFunctionPoint);
  }

  return (fpNotify)(pwcMsg);
}

__declspec(dllexport) void findLoadProcessImageBase() {
  HMODULE hLoadProcessImageBase = GetModuleHandle(NULL);
  if (!hLoadProcessImageBase) {
    MessageBox(NULL, L"Error getting module", L"findLoadProcessImageBase",
               MB_OK | MB_ICONWARNING);
  }
  // HANDLE hFile =
  //     CreateFile(L"C:\\Users\\a\\Temp\\x", GENERIC_READ | GENERIC_WRITE,
  //                FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS,
  //                FILE_ATTRIBUTE_NORMAL, NULL);
  CHAR abBuffer[64];
  ZeroMemory(abBuffer, 64);
  for (int i = 0; i < 63; i++) {
    abBuffer[63 - i] = (((ULONGLONG)(hLoadProcessImageBase) >> i) & 1) + '0';
  }
  DWORD dwBytesWritten = 0;
  WriteFile(hPipe, abBuffer, 64, &dwBytesWritten, NULL);
  // MessageBoxA(NULL, abBuffer, "findLoadProcessImageBase",
  //             MB_OK | MB_ICONASTERISK);
  // CloseHandle(hFile);
}
}