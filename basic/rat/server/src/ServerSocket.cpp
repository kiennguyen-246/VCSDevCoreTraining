#include "ServerSocket.hpp"

ServerSocket::ServerSocket(std::string __sIpv4Addr, int __iPort) {
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(__iPort);
  serverAddress.sin_addr.s_addr = inet_addr(__sIpv4Addr.c_str());
}

ServerSocket::~ServerSocket() {}

std::string ServerSocket::getServerIpv4Addr() {
  return std::string(inet_ntoa(serverAddress.sin_addr));
}

int ServerSocket::getServerPort() { return ntohs(serverAddress.sin_port); }

int ServerSocket::openSocket() {
  iFdServerSocket = socket(AF_INET, SOCK_STREAM, 0);

  if (bind(iFdServerSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) ==
      -1) {
    logEvent(std::format("bind() failed {}", errno), LOG_TYPE_ERROR);
    return errno;
  }

  //   std::cout << std::format("Server is listening on port {}",
  //   getServerPort());

  if (listen(iFdServerSocket, 5) == -1) {
    logEvent(std::format("listen() failed {}", errno), LOG_TYPE_ERROR);
    return errno;
  }

  fAcceptanceThread = std::async(&ServerSocket::acceptanceLoop, this);

  return 0;
}

int ServerSocket::sendMsgWithReply(std::string sDestination,
                                   std::string sMessage, std::string* psReply) {
  if (umClientSockets.find(sDestination) == umClientSockets.end()) {
    *psReply = std::format(
        "The address {} does not exisis in the list of active connections",
        sDestination);
    return 0;
  }

  int iFdClientSocket = umClientSockets[sDestination];

  int iResult = 0;
  iResult = send(iFdClientSocket, sMessage.c_str(),
                 sMessage.length() * sizeof(char), 0);

  // std::cout << iResult << " " << errno << "\n";
  if (iResult == -1) {
    logEvent(std::format("send() failed {}", errno), LOG_TYPE_ERROR);
    // std::cout << "Send failed " << errno << "\n";
    return errno;
  }

  char pcRecvBuffer[1024];
  int iRecvBufferLen = sizeof(pcRecvBuffer);
  memset(pcRecvBuffer, 0, sizeof(pcRecvBuffer));
  iResult = recv(iFdClientSocket, pcRecvBuffer, iRecvBufferLen, 0);

  // std::cout << iResult << " " << errno << "\n";
  if (iResult == 0) {
    umClientSockets.erase(sDestination);
    std::cout
        << "Received empty message from the client. Server will abort this "
           "connection.\n";
    logEvent(
        "Received empty message from the client. Server will abort this "
        "connection.",
        LOG_TYPE_WARNING);
    return -1;
  }
  if (iResult == -1) {
    logEvent(std::format("recv() failed {}", errno), LOG_TYPE_ERROR);
    // std::cout << "Receive failed " << errno << "\n";
    return errno;
  }

  *psReply = pcRecvBuffer;
  return 0;
}

int ServerSocket::closeSocket() {
  if (close(iFdServerSocket) == -1) {
    logEvent(std::format("close() failed {}", errno), LOG_TYPE_ERROR);
    return errno;
  }
  return 0;
}

int ServerSocket::acceptanceLoop() {
  while (1) {
    sockaddr_in clientAddress;
    socklen_t uiClientAddressLen = sizeof(clientAddress);
    int iFdClientSocket =
        accept(iFdServerSocket, (sockaddr*)&clientAddress, &uiClientAddressLen);
    if (iFdClientSocket == -1) {
      logEvent(std::format("accept() failed {}", errno), LOG_TYPE_ERROR);
      return errno;
    }

    std::string sClientAddr =
        std::format("{}:{}", inet_ntoa(clientAddress.sin_addr),
                    ntohs(clientAddress.sin_port));
    umClientSockets.insert({sClientAddr, iFdClientSocket});

    logEvent(std::format("Client {} successfully connected", sClientAddr,
                         getServerPort()),
             LOG_TYPE_INFO);
  }
  return 0;
}