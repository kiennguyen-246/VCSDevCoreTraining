#include <Windows.h>

#include <iostream>

int wmain() {
  DWORD dwPid;
  DWORD dwErr = 0;
  std::wcout << "PID: ";
  std::wcin >> dwPid;
  HANDLE hRemoteProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);
  WCHAR awcDllPath[] = TEXT("MessageBox.dll");
  LPVOID lpDllPathRemoteBuffer = VirtualAllocEx(
      hRemoteProc, NULL, sizeof(awcDllPath), MEM_COMMIT, PAGE_READWRITE);
  if (!lpDllPathRemoteBuffer) {
    dwErr = GetLastError();
    std::wcout << "VirtualAllocEx failed " << dwErr << "\n";
    return dwErr;
  }

  BOOL bRet = WriteProcessMemory(hRemoteProc, lpDllPathRemoteBuffer, awcDllPath,
                                 sizeof(awcDllPath), NULL);
  if (!bRet) {
    dwErr = GetLastError();
    std::wcout << "VirtualAllocEx failed " << dwErr << "\n";
    return dwErr;
  }

  HINSTANCE hKernel32 = GetModuleHandle(TEXT("Kernel32.dll"));
  if (!hKernel32) {
    dwErr = GetLastError();
    std::wcout << "GetModuleHandle failed " << dwErr << "\n";
    return dwErr;
  }

  FARPROC fpLoadLibrary = GetProcAddress(hKernel32, "LoadLibraryW");
  if (!fpLoadLibrary) {
    dwErr = GetLastError();
    std::wcout << "GetProcAddress failed " << dwErr << "\n";
    return dwErr;
  }

  DWORD dwRemoteTid = 0;
  HANDLE hRemoteThread = CreateRemoteThread(
      hRemoteProc, NULL, 0, (LPTHREAD_START_ROUTINE)fpLoadLibrary,
      lpDllPathRemoteBuffer, 0, &dwRemoteTid);
  if (!hRemoteThread) {
    dwErr = GetLastError();
    std::wcout << "CreateRemoteThread failed " << dwErr << "\n";
    return dwErr;
  }
  return 0;
}