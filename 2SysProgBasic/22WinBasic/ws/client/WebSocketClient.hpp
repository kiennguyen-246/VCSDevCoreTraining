#ifndef WEB_SOCKET_CLIENT_HPP
#define WEB_SOCKET_CLIENT_HPP

#include <winsock2.h>
#include <websocket.h>

#include <iostream>

#include "TransportManager.hpp"
#include "ClientHTTPSocket.hpp"

const int WSC_MAX_BUFFER_SIZE = TM_MAX_BUFFER_SIZE;
const int WSC_MAX_NUMBER_OF_BUFFERS = 2;

class WebSocketClient {
 public:
  WebSocketClient();
  ~WebSocketClient();

  //std::string wscGetHostName();

  //bool wscSetHostName(const std::string& sNewHostName);

  HRESULT wscInitialize();

  HRESULT wscPerformHandshake(std::wstring wsHost, std::wstring wsPort);

  HRESULT wscPerformDataExchange(std::wstring wsMsg);

  HRESULT wscDeleteHandle();

 private:
  std::string sFullHostName;

  WEB_SOCKET_HANDLE wshClient;
  //WEB_SOCKET_HANDLE wshServer;
  
  ClientHTTPSocket chs;

  TransportManager tm;

  HRESULT wscRunGetActionLoop();

  bool bIsHandshakeSuccessful;
};

#endif