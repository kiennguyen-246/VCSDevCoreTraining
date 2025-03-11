#include <iostream>

#include "ClientHTTPSocket.hpp"

const WCHAR HOSTNAME[] = L"192.168.0.104";
const WCHAR DEFAULT_PORT[] = L"8000";
const WCHAR MAX_BUFFER_SIZE = 1024;

int wmain(int argc, LPWSTR argv[]) {
  /*ClientSocket cs;
  HRESULT hr;

  hr = cs.getSocketInitResult();
  if (hr != S_OK) {
    return 1;
  }

  hr = cs.connectToHost(HOSTNAME, DEFAULT_PORT);
  if (hr != S_OK) {
    return 1;
  }

  wprintf(L"Successfully connected to the server at %ws:%ws\n", HOSTNAME,
          DEFAULT_PORT);
  getchar();

  cs.sendMsg(L"Ambatukam\n");
  Sleep(1000);
  cs.sendMsg(L"Ambatukam\n");
  Sleep(1000);
  cs.sendMsg(L"Ambatukam\n");
  Sleep(1000);
  cs.sendMsg(L"Ambatukam\n");
  Sleep(1000);
  cs.sendMsg(L"Ambatukam\n");
  Sleep(1000);
  cs.sendMsg(L"Ambatukam\n");
  Sleep(1000);
  cs.sendMsg(L"Ambatukam\n");
  Sleep(1000);

  hr = cs.disconnectFromCurrentHost();
  if (hr != S_OK) {
    return 1;
  }*/

  ClientHTTPSocket chs;
  HRESULT hr;


  hr = chs.getSocketInitResult();
  if (hr != S_OK) {
    return hr;
  }

  hr = chs.connectToHost(HOSTNAME, DEFAULT_PORT);
  if (hr != S_OK) {
    return hr;
  }

  wprintf(L"Successfully connected to the server at %ws:%ws\n", HOSTNAME,
          DEFAULT_PORT);
  getchar();

  chs.sendGETRequest();
  chs.receiveResponse();

  wprintf(L"Response status is %d", chs.getResponseHTTPStatus());

  return 0;
}