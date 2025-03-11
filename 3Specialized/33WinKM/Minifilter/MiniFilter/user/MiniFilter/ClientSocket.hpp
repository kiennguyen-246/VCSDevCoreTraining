#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <winsock2.h>
#include <ws2tcpip.h>

#include <iostream>
#include <vector>
#include <mutex>

class ClientSocket {
 private:
  HRESULT hrSocketInitResult;

 protected:
  SOCKET connectSocket;

 public:
  ClientSocket();
  ~ClientSocket();

  HRESULT getSocketInitResult();

  HRESULT connectToHost(std::wstring wsHost, std::wstring wsPort);

  HRESULT disconnectFromCurrentHost();

  HRESULT sendMsg(std::wstring* pwsMsg);

  HRESULT receiveMsg(std::wstring* pwsMsg);

  HRESULT sendData(LPSTR pcBuffer, PULONG puiBufferLengthSend);

  HRESULT receiveData(LPSTR pcBuffer, PULONG puiBufferMaximumLength,
                      PULONG puiBufferLengthReceived);
};

#endif