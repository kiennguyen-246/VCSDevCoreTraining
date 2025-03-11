#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <format>
#include <future>
#include <iostream>
#include <unordered_map>
#include <vector>

#include "EventLogger.hpp"

class ServerSocket {
 public:
  ServerSocket(std::string __sIpv4Addr, int __iPort);
  ~ServerSocket();

  std::string getServerIpv4Addr();

  std::vector<std::string> getClientsAddresses() {
    std::vector<std::string> vRet;
    for (auto p : umClientSockets) {
      vRet.push_back(p.first);
    }
    return vRet;
  }

  int getServerPort();

  int openSocket();

  int sendMsgWithReply(std::string sDestination, std::string sMessage,
                       std::string* psReply);

  int closeSocket();

 private:
  std::unordered_map<std::string, int> umClientSockets;

  int iFdServerSocket;

  sockaddr_in serverAddress;

  std::future<int> fAcceptanceThread;

  int acceptanceLoop();
};