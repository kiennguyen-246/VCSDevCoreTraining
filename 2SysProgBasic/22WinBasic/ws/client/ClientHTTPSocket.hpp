#ifndef CLIENT_HTTP_SOCKET_HPP
#define CLIENT_HTTP_SOCKET_HPP

#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>

#include "ClientSocket.hpp"
#include "utils.hpp"

const int CHS_MAX_MESSAGE_SIZE = 2048;
const int CHS_CONTENT_LENGTH = 1024;

class ClientHTTPSocket : public ClientSocket {
 private:
  std::unordered_map<std::string, std::string> umssRequestHeaders;

  std::unordered_map<std::string, std::string> umssResponseHeaders;

  CHAR pcRequestBuffer[CHS_CONTENT_LENGTH];

  CHAR pcResponseBuffer[CHS_CONTENT_LENGTH];

  int iResponseHTTPStatus;

 public:
  ClientHTTPSocket();
  ~ClientHTTPSocket();

  HRESULT addRequestHeader(std::string key, std::string value);

  HRESULT removeRequestHeader(std::string key);

  std::unordered_map<std::string, std::string> getResponseHeaders();

  int getResponseHTTPStatus();

  HRESULT connectToHost(std::wstring wsHost, std::wstring wsPort);

  HRESULT disconnectFromCurrentHost();

  HRESULT sendGETRequest();

  HRESULT receiveResponse();
};

#endif  // !CLIENT_HTTP_SOCKET_HPP
