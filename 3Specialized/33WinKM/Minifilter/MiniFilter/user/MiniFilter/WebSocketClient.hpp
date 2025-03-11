#ifndef WEB_SOCKET_CLIENT_HPP
#define WEB_SOCKET_CLIENT_HPP

#include <winsock2.h>
#include <websocket.h>

#include <iostream>
#include <queue>
#include <future>

#include "ClientHTTPSocket.hpp"

const int WSC_MAX_BUFFER_SIZE = 1024;
const int WSC_MAX_NUMBER_OF_BUFFERS = 2;

class WebSocketClient {
 public:
  WebSocketClient();
  ~WebSocketClient();

  //std::string wscGetHostName();

  //bool wscSetHostName(const std::string& sNewHostName);

  HRESULT init();

  HRESULT handshake(std::wstring wsHost, std::wstring wsPort);

  HRESULT queueMsg(std::wstring wsMsg);

  HRESULT cleanup();

  bool isHandshakeSuccessful();

  HRESULT setShouldStop();

 private:
  std::string sFullHostName;

  std::wstring wsCurrentServerHost;
  
  std::wstring wsCurrentServerPort;

  WEB_SOCKET_HANDLE wshClient;
  //WEB_SOCKET_HANDLE wshServer;
  
  ClientHTTPSocket chs;

  bool bIsHandshakeSuccessful;

  volatile bool bShouldStop;

  std::queue<std::wstring> qwsMsgQueue;

  std::future<HRESULT> fSendMsgFuture;

  HRESULT runGetActionLoop();

  HRESULT send();

  HRESULT reconnect();
  
};

#endif