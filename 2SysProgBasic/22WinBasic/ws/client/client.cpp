#include <format>

#include "WebSocketClient.hpp"

const WCHAR HOSTNAME[] = L"192.168.88.137";
// const WCHAR HOSTNAME[] = L"192.168.150.129";
const WCHAR DEFAULT_PORT[] = L"55555";
const WCHAR MAX_BUFFER_SIZE = 1024;

int wmain(int argc, LPWSTR argv[]) {
  HRESULT hr = S_OK;
  WebSocketClient wsc;

  // std::string sFullHostName = "192.168.0.103:9000";
  // wsc.wscSetHostName(sFullHostName); 

  hr = wsc.wscInitialize();
  if (FAILED(hr)) {
    std::wcout << L"Failed to initialize socket handles\n";
    wsc.wscDeleteHandle();
    return 0;
  }
  std::wcout << L"Successfully initializing socket handles\n";

  hr = wsc.wscPerformHandshake(HOSTNAME, DEFAULT_PORT);
  if (FAILED(hr)) {
    std::wcout << L"Hanshaking failed\n ";

    wsc.wscDeleteHandle();
    return 0;
  }
  std::wcout << L"Hanshaking successful. Connection established.\n ";

  std::wstring wsMsg;

  /*wsMsg = L"Ambatukam";
  hr = wsc.wscPerformDataExchange(wsMsg);
  if (FAILED(hr)) {
    std::wcout << "Failed to exchange data\n";
    wsc.wscDeleteHandle();
    return 0;
  }
  wsMsg = L"Ambasing";
  hr = wsc.wscPerformDataExchange(wsMsg);
  if (FAILED(hr)) {
    std::wcout << "Failed to iexchange data\n";
    wsc.wscDeleteHandle();
    return 0;
  }
  wsMsg = L"Ambatunat";
  hr = wsc.wscPerformDataExchange(wsMsg);
  if (FAILED(hr)) {
    std::wcout << "Failed to exchange data\n";
    wsc.wscDeleteHandle();
    return 0;
  }*/

  for (int i = 1; i < 300; i++) {
    wsMsg =
        L"{ 'time': '24/09/2024 15:05:57', 'fileHandleDirectory': '\\\\\\\\', "
        L"'filterName': 'MiniFilter', 'fileHandleVolumeDosName': 'E:', "
        L"'eventObjType': 'file', 'action': 'open', 'fileHandleType': "
        L"'directory', 'operationStatus': '0x00000000', "
        L"'isAttemptingOverwrite': '0' }";

    hr = wsc.wscPerformDataExchange(wsMsg);
    if (FAILED(hr)) {
      std::wcout << "Failed to exchange data\n";
      wsc.wscDeleteHandle();
      return 0;
    }
    // Sleep(1000);
  }

  return 0;
}