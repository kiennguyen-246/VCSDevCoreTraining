
#include "Client.hpp"

Client* Client::getInstance() {
  if (pClientInstance == nullptr) {
    pClientInstance = new Client();
  }
  return pClientInstance;
}

int Client::init(std::string __sServerIpv4Addresss, int __iServerPort) {
  sServerIpv4Address = __sServerIpv4Addresss;
  iServerPort = __iServerPort;
  pcm = CommandManager::getInstance();

  pcs = new ClientSocket();
  int iResult = pcs->connectToSocket(sServerIpv4Address, iServerPort);
  if (iResult) {
    return iResult;
  }
  std::cout << std::format("Connected to server at {}:{}\n", sServerIpv4Address,
                           iServerPort);
  return 0;
}

int Client::loop() {
  while (1) {
    std::string sCmdFromServer;
    std::string sOutput;

    if (pcs->recvMsg(&sCmdFromServer)) {
      return -1;
    }
    std::cout << std::format("Received command from server: {}\n",
                             sCmdFromServer);

    command(sCmdFromServer, &sOutput);
    std::cout << std::format("Result: {}\n", sOutput);
    if (pcs->sendMsg(sOutput)) {
      return -1;
    }
  }
  return 0;
}

int Client::stop() { return pcs->closeSocket(); }

Client* Client::pClientInstance = nullptr;

Client::Client() {}

Client::~Client() {}

int Client::command(std::string sInput, std::string* psOutput) {
  std::istringstream issInput(sInput);
  int iCnt = 0;
  std::string sCmd = "";
  std::string sArgs = "";
  bool bIsTruncated;
  bool bIsRun;
  issInput >> sCmd >> sArgs;

  if (sCmd == "pwd") {
    pcm->pwd(psOutput, &bIsTruncated);
    bIsRun = true;
  }
  if (sCmd == "cd") {
    pcm->cd(sArgs, psOutput, &bIsTruncated);
    bIsRun = true;
  }
  if (sCmd == "ls") {
    //   std::cout << std::format("sCmd = {}, sArgs = {}\n", sCmd, sArgs);
    pcm->ls(sArgs, psOutput, &bIsTruncated);
    bIsRun = true;
  }
  if (sCmd == "cat") {
    pcm->cat(sArgs, psOutput, &bIsTruncated);
    bIsRun = true;
  }
  if (sCmd == "ps") {
    pcm->ps(psOutput, &bIsTruncated);
    bIsRun = true;
  }
  if (sCmd == "kill") {
    pcm->kill(sArgs, psOutput, &bIsTruncated);
    bIsRun = true;
  }
  if (bIsRun) {
    if (bIsTruncated) {
      *psOutput += "... (truncated)";
    }
  } else {
    *psOutput = "Command not supported.";
  }
  *psOutput += " ";
  return 0;
}