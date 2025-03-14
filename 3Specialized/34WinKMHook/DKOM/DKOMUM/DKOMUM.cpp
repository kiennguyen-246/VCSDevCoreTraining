#include <Windows.h>
#include <fltUser.h>

#include <iostream>

#define DKOM_COM_PORT_NAME L"\\DKOMCOM"

INT wmain() {
  DWORD dwPid;
  std::wcout << L"PID: ";
  std::wcin >> dwPid;

  HRESULT hr;
  HANDLE hComPort;
  hr = FilterConnectCommunicationPort(DKOM_COM_PORT_NAME, 0, NULL, 0, NULL,
                                      &hComPort);
  if (hr != S_OK) {
    std::wcout << L"FilterConnectCommunicationPort failed " << hr << "\n";
    return 1;
  }

  FILTER_MESSAGE_HEADER messageHeader;
  BYTE abMessageBuffer[256];
  BYTE abReplyBuffer[256];
  ZeroMemory(abMessageBuffer, sizeof(abMessageBuffer));
  ZeroMemory(abReplyBuffer, sizeof(abReplyBuffer));
  DWORD dwBytesReturned = 0;
  for (DWORD i = 0; i < 64; i++) {
    abMessageBuffer[i] = ((dwPid >> i) & 1) + '0';
  }
  hr = FilterSendMessage(hComPort, abMessageBuffer, 256, abReplyBuffer, 256,
                         &dwBytesReturned);
  if (hr != S_OK) {
    std::wcout << L"FilterSendMessage failed " << hr << "\n";
    return 1;
  }

  if (abReplyBuffer[0] == 'o') {
    std::wcout << L"Operation successful" << "\n";
  } else {
    std::wcout << L"Operation failed " << "\n";
  }

  CloseHandle(hComPort);
  return 0;
}
