#include <future>
#include <queue>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "ServerSocket.hpp"

class Server {
 public:
  static Server* getInstance();

  int init(std::string __sServerIpv4Addresss, int __iServerPort);

  int loop();

  int stop();

 private:
  Server();
  ~Server();

  static Server* pServerInstance;

  std::string sServerIpv4Address;

  int iServerPort;

  ServerSocket* pss;

  int command(std::string sCmd, std::string sIpv4Addr, std::string* psRes);
};