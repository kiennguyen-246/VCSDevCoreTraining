#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <format>
#include <iostream>

class ClientSocket {
 public:
  ClientSocket();
  ~ClientSocket();

  std::string getServerIpv4Addr();

  int getServerPort();

  std::string getClientIpv4Addr();

  int getClientPort();

  int connectToSocket(std::string sIpv4Addr, int iPort);

  int sendMsg(std::string sMsg);

  int recvMsg(std::string* psMsg);

  int closeSocket();

 private:
  int iFdClientSocket;

  sockaddr_in serverAddress;

  sockaddr_in clientAddress;
};