#include "ClientSocket.hpp"

ClientSocket::ClientSocket() {}

ClientSocket::~ClientSocket() {}

std::string ClientSocket::getServerIpv4Addr() {
  return std::string(inet_ntoa(serverAddress.sin_addr));
}

int ClientSocket::getServerPort() { return ntohs(serverAddress.sin_port); }

std::string ClientSocket::getClientIpv4Addr() {
  return std::string(inet_ntoa(clientAddress.sin_addr));
}

int ClientSocket::getClientPort() { return ntohs(clientAddress.sin_port); }

int ClientSocket::connectToSocket(std::string sIpv4Addr, int iPort) {
  iFdClientSocket = socket(AF_INET, SOCK_STREAM, 0);

  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(iPort);
  serverAddress.sin_addr.s_addr = inet_addr(sIpv4Addr.c_str());
  if (connect(iFdClientSocket, (sockaddr*)&serverAddress,
              sizeof(serverAddress)) == -1) {
    std::cout << std::format("Connect to server socket at port {} failed {}\n",
                             iPort, errno);
    return errno;
  }
  return 0;
}

int ClientSocket::sendMsg(std::string sMsg) {
  sMsg += "~";
  if (send(iFdClientSocket, sMsg.c_str(), sMsg.size(), 0) == -1) {
    std::cout << std::format("Failed to send some messages to the server {}\n",
                             errno);
    return errno;
  }
  return 0;
}

int ClientSocket::recvMsg(std::string* psMsg) {
  char pcRecvBuffer[1024];
  int iRecvBufferLen = sizeof(pcRecvBuffer);
  memset(pcRecvBuffer, 0, sizeof(pcRecvBuffer));
  if (recv(iFdClientSocket, pcRecvBuffer, iRecvBufferLen, 0) == -1) {
    std::cout << std::format(
        "Failed to receive some messages from the server {}\n", errno);
    return errno;
  }
  *psMsg = pcRecvBuffer;
  return 0;
}

int ClientSocket::closeSocket() {
  if (close(iFdClientSocket) == -1) {
    std::cout << std::format("Failed to close socket {}\n", errno);
    return errno;
  }
  return 0;
}