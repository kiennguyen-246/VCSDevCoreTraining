#include <future>
#include <mutex>
#include <vector>

#include "ClientSocket.hpp"
#include "CommandManager.hpp"

class Client {
 public:
  static Client* getInstance();

  int init(std::string __sServerIpv4Addresss, int __iServerPort);

  int loop();

  int stop();

 private:
  static Client* pClientInstance;

  ClientSocket* pcs;

  CommandManager* pcm;

  std::string sServerIpv4Address;

  int iServerPort;

  Client();
  ~Client();

  int command(std::string sInput, std::string* psOutput);
};