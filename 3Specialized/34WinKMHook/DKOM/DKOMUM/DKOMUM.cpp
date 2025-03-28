#include <Windows.h>
#include <fltUser.h>

#include <iostream>

#define DKOM_COM_PORT_NAME L"\\DKOMCOM"

INT wmain() {
  DWORD dwPID = GetCurrentProcessId();
   std::wcout << L"PID: ";
   std::wcin >> dwPID;

  HRESULT hr;
  HANDLE hComPort;
  hr = FilterConnectCommunicationPort(DKOM_COM_PORT_NAME, 0, NULL, 0, NULL,
                                      &hComPort);
  if (hr != S_OK) {
    std::wcout << L"FilterConnectCommunicationPort failed " << hr << "\n";
    return 1;
  }

  BYTE abMessageBuffer[256];
  BYTE abReplyBuffer[256];
  ZeroMemory(abMessageBuffer, sizeof(abMessageBuffer));
  ZeroMemory(abReplyBuffer, sizeof(abReplyBuffer));
  hr = FilterGetMessage(hComPort, (PFILTER_MESSAGE_HEADER)abReplyBuffer,
                        sizeof(FILTER_MESSAGE_HEADER) + 3, NULL);
  if (hr != S_OK) {
    std::wcout << L"FilterGetMessage failed " << hr << "\n";
    CloseHandle(hComPort);
    return 1;
  }

  // for (int i = 0; i < 256; i++) {
  //   std::cout << std::hex << (ULONG)abReplyBuffer[i] << " ";
  // }
  // std::cout << "\n";
  // MessageBox(NULL, L"Press OK to continue", L"DKOMUM",
  //            MB_OK | MB_ICONINFORMATION);

  if (abReplyBuffer[sizeof(FILTER_MESSAGE_HEADER)] == 'o') {
    std::wcout << L"Connection established" << "\n";
  } else {
    std::wcout << L"Connection failed " << "\n";
    MessageBox(NULL, L"Connection failed", L"DKOMUM", MB_OK | MB_ICONSTOP);
    CloseHandle(hComPort);
    return 1;
  }

  DWORD dwBytesReturned = 0;
  CopyMemory(abMessageBuffer, abReplyBuffer, sizeof(FILTER_REPLY_HEADER));
  for (DWORD i = 0; i < sizeof(ULONGLONG); i++) {
    abMessageBuffer[i] = 0;
  }
  for (DWORD i = 0; i < 64; i++) {
    abMessageBuffer[i + sizeof(FILTER_REPLY_HEADER)] = ((dwPID >> i) & 1) + '0';
  }
  // for (int i = 0; i < 256; i++) {
  //   std::cout << std::hex << (ULONG)abMessageBuffer[i] << " ";
  // }
  // std::cout << "\n";
  // MessageBox(NULL, L"Press OK to continue", L"DKOMUM",
  //            MB_OK | MB_ICONINFORMATION);

  hr = FilterReplyMessage(hComPort, (PFILTER_REPLY_HEADER)abMessageBuffer,
                          sizeof(FILTER_REPLY_HEADER) + 64);
  if (hr != S_OK) {
    std::wcout << L"FilterReplyMessage failed " << hr << "\n";
    CloseHandle(hComPort);
    return 1;
  }

  ZeroMemory(abMessageBuffer, sizeof(abMessageBuffer));
  ZeroMemory(abReplyBuffer, sizeof(abReplyBuffer));
  hr = FilterGetMessage(hComPort, (PFILTER_MESSAGE_HEADER)abReplyBuffer,
                        sizeof(FILTER_MESSAGE_HEADER) + 3, NULL);
  if (hr != S_OK) {
    std::wcout << L"FilterGetMessage failed " << hr << "\n";
    CloseHandle(hComPort);
    return 1;
  }

  // for (int i = 0; i < 256; i++) {
  //   std::cout << std::hex << (ULONG)abReplyBuffer[i] << " ";
  // }
  // std::cout << "\n";
  // MessageBox(NULL, L"Press OK to continue", L"DKOMUM",
  //            MB_OK | MB_ICONINFORMATION);

  if (abReplyBuffer[sizeof(FILTER_MESSAGE_HEADER)] == 'o') {
    std::wcout << L"Operation succeeded" << "\n";
  } else {
    std::wcout << L"Operation failed " << "\n";
    MessageBox(NULL, L"Operation failed", L"DKOMUM", MB_OK | MB_ICONSTOP);
    CloseHandle(hComPort);
    return 1;
  }

  MessageBox(NULL, L"Operation succeeded", L"DKOMUM",
             MB_OK | MB_ICONINFORMATION);

  CloseHandle(hComPort);

  //Sleep(1000000);
  return 0;
}
