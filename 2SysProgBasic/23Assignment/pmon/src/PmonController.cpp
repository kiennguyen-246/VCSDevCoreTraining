#include "pmon/PmonController.hpp"

PmonController* PmonController::getInstance() {
  if (pInstance == NULL) {
    pInstance = new PmonController();
  }
  return pInstance;
}

int PmonController::run() {
  int iResult = 0;

  iResult = init();
  if (iResult) {
    return iResult;
  }

  iResult = mainThreadMain();
  if (iResult) {
    // return iResult;
  }

  iResult = clean();
  if (iResult) {
    return iResult;
  }

  return iResult;
}

PmonController::PmonController() {
#if __linux__
    
#endif

#if _WIN32
  hkConfig = HKEY_CURRENT_USER;
  hNpSendEvent = INVALID_HANDLE_VALUE;
  hNpRecvConfigChange = INVALID_HANDLE_VALUE;
#endif

  bStop = false;
}

PmonController::~PmonController() {}

#ifdef __linux__
int PmonController::init() {
  int iResult = 0;

  logEvent("Initializing controller", LOG_TYPE_INFO);

  iResult = FastSystem::mkdir(CONFIG_DIR_PATH);
  if (iResult) {
    logEvent(std::format("FastSystem::mkdir failed {}\n", iResult),
             LOG_TYPE_ERROR);
    return iResult;
  }

  bStop = 0;

  std::vector<std::string> vsConfigFilePaths;
  iResult = FastSystem::ls(CONFIG_DIR_PATH, vsConfigFilePaths);
  if (iResult) {
    logEvent(std::format("FastSystem::ls failed {}\n", iResult),
             LOG_TYPE_ERROR);
    return iResult;
  }

  for (auto sConfigFilePath : vsConfigFilePaths) {
    auto iExtensionPos = sConfigFilePath.find(".cfg");
    if (iExtensionPos == std::string::npos) {
      continue;
    }

    auto sProcName = sConfigFilePath.substr(0, iExtensionPos);
    std::vector<int> viPids;
    iResult = FastSystem::pgrep(sProcName, viPids, "-x");
    if (iResult) {
      logEvent(std::format("FastSystem::pgrep failed {}\n", iResult),
               LOG_TYPE_ERROR);
      return iResult;
    }
    for (auto iPid : viPids) {
      mProcsByPid[iPid] = new Process(iPid);
      mProcsByPid[iPid]->updateInfo();
      // std::cout << mProcsByPid[iPid]->toString() << "\n";
    }
  }

  logEvent("Successfully initialized process objects", LOG_TYPE_INFO);

  while (!qEvents.empty()) {
    qEvents.pop();
  }

  mq_attr mqa;
  memset(&mqa, 0, sizeof(mq_attr));
  mqa.mq_maxmsg = 50;
  mqa.mq_msgsize = 2048;
  mqdSendEvent = mq_open(MQ_SEND_EVENT_NAME.c_str(),
                         O_CREAT | O_WRONLY | O_NONBLOCK, 0777, mqa);
  if (mqdSendEvent == (mqd_t)-1) {
    logEvent(
        std::format("mq_open() failed {} for {}\n", errno, MQ_SEND_EVENT_NAME),
        LOG_TYPE_ERROR);
    return errno;
  }

  mqdRecvConfigChange = mq_open(MQ_RECV_CONFIG_CHANGE_NAME.c_str(),
                                O_CREAT | O_RDONLY | O_NONBLOCK, 0777, mqa);
  if (mqdRecvConfigChange == (mqd_t)-1) {
    logEvent(std::format("mq_open() failed {} for {}\n", errno,
                         MQ_RECV_CONFIG_CHANGE_NAME),
             LOG_TYPE_ERROR);
    return errno;
  }

  logEvent("Successfully opened backend message queues", LOG_TYPE_INFO);

  fMonitorProcessThread =
      std::async(&PmonController::monitorProcessesThreadMain, this);
  logEvent("Successfully started backend monitoring processes thread ",
           LOG_TYPE_INFO);

  fSendEventThread = std::async(&PmonController::sendEventThreadMain, this);
  logEvent("Successfully started event sending thread", LOG_TYPE_INFO);

  fHandleConfigChangeThread =
      std::async(&PmonController::handleConfigChangeThreadMain, this);
  logEvent("Successfully started configuration changes handling thread",
           LOG_TYPE_INFO);

  return 0;
}

int PmonController::mainThreadMain() {
  while (!bStop) {
    // system("clear");
    std::cout << ">";
    std::string sCmd;
    std::cin >> sCmd;
    if (sCmd == "stop") {
      bStop = 1;
    }
  }

  return 0;
}

int PmonController::clean() {
  bStop = 1;
  int iResult = 0;

  int iResult1 = fMonitorProcessThread.get();
  logEvent(std::format("Process monitoring thread terminated with status {}\n ",
                       iResult1),
           LOG_TYPE_INFO);

  int iResult2 = fSendEventThread.get();
  logEvent(
      std::format("Communication thread terminated with status {}\n", iResult2),
      LOG_TYPE_INFO);

  int iResult3 = fHandleConfigChangeThread.get();
  logEvent(
      std::format(
          "Configuration change handling thread terminated with status {}\n",
          iResult3),
      LOG_TYPE_INFO);

  logEvent("Successfully terminated backend threads", LOG_TYPE_INFO);

  iResult = mq_close(mqdSendEvent);
  if (iResult) {
    logEvent(
        std::format("mq_close() failed {} for {}\n", errno, MQ_SEND_EVENT_NAME),
        LOG_TYPE_ERROR);
    return errno;
  }

  iResult = mq_close(mqdRecvConfigChange);
  if (iResult) {
    logEvent(std::format("mq_close() failed {} for {}\n", errno,
                         MQ_RECV_CONFIG_CHANGE_NAME),
             LOG_TYPE_ERROR);
    return errno;
  }

  logEvent("Successfully closed backend message queues", LOG_TYPE_INFO);

  for (auto p : mProcsByPid) {
    // std::cout << "About to delete p.second";
    delete p.second;
  }
  mProcsByPid.clear();

  logEvent("Successfully freed backend process objects", LOG_TYPE_INFO);

  logEvent("Successfully shut down the backend controller", LOG_TYPE_INFO);

  return 0;
}

int PmonController::monitorProcessesThreadMain() {
  int iResult = 0;

  while (!bStop) {
    sleep(3);

    std::vector<std::string> vsConfigFilePaths;
    iResult = FastSystem::ls(CONFIG_DIR_PATH, vsConfigFilePaths);
    if (iResult) {
      logEvent(std::format("FastSystem::ls failed {}\n", iResult),
               LOG_TYPE_ERROR);
      return iResult;
    }

    for (auto sConfigFilePath : vsConfigFilePaths) {
      std::string sConfigDirPath = Utils::resolveHomeDir(CONFIG_DIR_PATH);
      sConfigFilePath = sConfigDirPath + sConfigFilePath;

      auto iExtensionPos = sConfigFilePath.find(".cfg");
      if (iExtensionPos == std::string::npos) {
        continue;
      }

      auto sProcName = sConfigFilePath.substr(
          sConfigDirPath.size(), iExtensionPos - sConfigDirPath.size());
      // std::cout << sProcName << "\n";
      std::vector<int> viPids;
      iResult = FastSystem::pgrep(sProcName, viPids, "-x");
      if (iResult) {
        logEvent(std::format("FastSystem::pgrep failed {}\n", iResult),
                 LOG_TYPE_ERROR);
        return iResult;
      }
      if (viPids.empty()) {
        continue;
      }
      for (auto iPid : viPids) {
        if (mProcsByPid[iPid] == NULL) {
          mProcsByPid[iPid] = new Process(iPid);
        }
        mProcsByPid[iPid]->updateInfo();
        // std::cout << mProcsByPid[iPid]->toString() << "\n";
      }

      // std::cout << sConfigFilePath << "\n";

      Configuration cfg;
      iResult = parseConfigFile(sConfigFilePath, cfg);
      if (iResult) {
        logEvent(
            std::format("PmonController::parseConfigFile failed {}\n", iResult),
            LOG_TYPE_ERROR);
        return iResult;
      }
      // std::cout << cfg.toString() << "\n";

      for (auto iPid : viPids) {
        auto iViolationInfo =
            OverloadEvent::getViolationInfo(*mProcsByPid[iPid], cfg);
        // std::cout << iViolationInfo << "\n";
        if (iViolationInfo) {
          Event* evt = new OverloadEvent(*mProcsByPid[iPid], cfg);
          // std::cout << evt->toString() << "\n";
          qEvents.push(evt);
        }
      }
    }
  }
  // fHandleConfigChangeThread.get();
  return iResult;
}

int PmonController::sendEventThreadMain() {
  int iResult = 0;

  while (!bStop) {
    mtx.lock();
    if (qEvents.empty()) {
      mtx.unlock();
      continue;
    }
    mtx.unlock();

    auto pEvent = qEvents.front();

    iResult = mq_send(mqdSendEvent, pEvent->toString().c_str(),
                      pEvent->toString().size(), 0);
    if (iResult) {
      int iErr = errno;
      if (iErr == EAGAIN) {
        logEvent(
            std::format("Sending message queue is full. Retrying in 1s.\n"),
            LOG_TYPE_WARNING);

      } else {
        logEvent(std::format("mq_send() failed {} for {}. Retrying in 1s.\n",
                             iErr, MQ_SEND_EVENT_NAME),
                 LOG_TYPE_ERROR);
      }
      sleep(1);
    } else {
      // std::cout << "Send ok\n";
      // std::cout << "About to delete event id = " << pEvent->getId() << "\n";
      delete pEvent;
      // std::cout << "Successfully deleted\n";
      // std::cout << qEvents.size() << "\n";
      qEvents.pop();
      // std::cout << qEvents.size() << "\n";
    }
  }
  // fHandleConfigChangeThread.get();
  // std::cout << "Run here\n";
  return iResult;
}

int PmonController::handleConfigChangeThreadMain() {
  int iResult = 0;

  while (!bStop) {
    MINI_CONFIGURATION mcfg;

    mq_attr mqa;
    mq_getattr(mqdRecvConfigChange, &mqa);

    char pcRecvBuffer[mqa.mq_msgsize];
    memset(pcRecvBuffer, 0, sizeof(pcRecvBuffer));
    iResult = mq_receive(mqdRecvConfigChange, pcRecvBuffer, mqa.mq_msgsize, 0);
    if (iResult == -1) {
      int iErr = errno;
      if (iErr != EAGAIN) {
        logEvent(std::format("mq_receive() failed {} for {}. Retrying in 1s.\n",
                             iErr, MQ_RECV_CONFIG_CHANGE_NAME),
                 LOG_TYPE_ERROR);
      }
      sleep(1);
    } else {
      // std::cout << "About to copy buffer to cfg object\n";
      memcpy(&mcfg, pcRecvBuffer, sizeof(MINI_CONFIGURATION));
      Configuration cfg(mcfg);
      // std::cout << "Copied successful\n";
      // std::cout << iResult << " " << sizeof(Configuration) << "\n";
      // for (int i = 0; i < 64; i++) std::cout << (int)pcRecvBuffer[i] << " ";
      // std::cout << "\n";
      // std::cout << "Request: " << cfg.toString() << "\n";
      if (cfg.getName().front() == '!') {
        iResult = handleDeleteConfig(cfg);
        if (iResult) {
          logEvent(std::format("handleDeleteConfig() failed {}.\n", errno),
                   LOG_TYPE_ERROR);
        }
      } else {
        iResult = handleCreateConfig(cfg);
        if (iResult) {
          logEvent(std::format("handleCreateConfig() failed {}.\n", errno),
                   LOG_TYPE_ERROR);
        }
      }
    }
  }
  // fSendEventThread.get();
  // std::cout << iResult;
  return 0;
}

int PmonController::handleCreateConfig(const Configuration& cfg) {
  int iResult = 0;

  std::string sConfigFilePath =
      Utils::resolveHomeDir(CONFIG_DIR_PATH) + cfg.getName() + ".cfg";

  while (sOpeningFiles.find(sConfigFilePath) != sOpeningFiles.end()) {
    sleep(1);
  }
  mtx.lock();
  sOpeningFiles.insert(sConfigFilePath);
  mtx.unlock();
  std::ofstream ofs(sConfigFilePath);
  if (ofs.is_open()) {
    ofs << cfg.toString();
    ofs.close();
  } else {
    logEvent(std::format("Cannot open config file {}.\n", sConfigFilePath),
             LOG_TYPE_ERROR);

    mtx.lock();
    sOpeningFiles.erase(sConfigFilePath);
    mtx.unlock();
    return -1;
  }

  mtx.lock();
  sOpeningFiles.erase(sConfigFilePath);
  mtx.unlock();

  return iResult;
}

int PmonController::handleDeleteConfig(const Configuration& cfg) {
  int iResult = 0;

  std::string sConfigFilePath =
      Utils::resolveHomeDir(CONFIG_DIR_PATH) + cfg.getName().substr(1) + ".cfg";

  while (sOpeningFiles.find(sConfigFilePath) != sOpeningFiles.end()) {
    sleep(1);
  }
  mtx.lock();
  sOpeningFiles.insert(sConfigFilePath);
  mtx.unlock();

  iResult = FastSystem::rm(sConfigFilePath);
  if (iResult) {
    logEvent(std::format("FastSystem::pgrep failed {}\n", iResult),
             LOG_TYPE_ERROR);
    mtx.lock();
    sOpeningFiles.erase(sConfigFilePath);
    mtx.unlock();
    return iResult;
  }

  mtx.lock();
  sOpeningFiles.erase(sConfigFilePath);
  mtx.unlock();

  std::vector<int> viPids;
  iResult = FastSystem::pgrep(cfg.getName().substr(1), viPids, "-x");
  if (iResult) {
    logEvent(std::format("FastSystem::pgrep failed {}\n", iResult),
             LOG_TYPE_ERROR);
    return iResult;
  }
  for (auto iPid : viPids) {
    // std::cout << "About to delete mProcsByPid[iPid]\n";
    delete mProcsByPid[iPid];
    mProcsByPid.erase(iPid);
  }

  return iResult;
}

int PmonController::parseConfigFile(const std::string sConfigFilePath,
                                    Configuration& cfg) {
  int iResult = 0;

  while (sOpeningFiles.find(sConfigFilePath) != sOpeningFiles.end()) {
    sleep(1);
  }
  mtx.lock();
  sOpeningFiles.insert(sConfigFilePath);
  mtx.unlock();

  std::ifstream ifs(sConfigFilePath);
  if (ifs.is_open()) {
    std::string str = "";
    while (ifs >> str) {
      // std::cout << str << "\n";
      if (str == "\"procName\":") {
        std::string sKey;
        std::string sValue;
        std::string sName;
        double dCpu;
        double dRam;
        double dDisk;
        double dNet;

        ifs >> sValue;
        sValue.pop_back();
        sValue.pop_back();
        sValue = sValue.substr(1);
        sName = sValue;

        ifs >> sKey >> sValue;
        sValue.pop_back();
        dCpu = atof(sValue.c_str());

        ifs >> sKey >> sValue;
        sValue.pop_back();
        dRam = atof(sValue.c_str());

        ifs >> sKey >> sValue;
        sValue.pop_back();
        dDisk = atof(sValue.c_str());

        ifs >> sKey >> sValue;
        sValue.pop_back();
        dNet = atof(sValue.c_str());

        cfg = Configuration(sName, dCpu, dRam, dDisk, dNet);
      }
    }
    ifs.close();
  } else {
    logEvent(std::format("Cannot open config file {}\n", sConfigFilePath),
             LOG_TYPE_ERROR);
    mtx.lock();
    sOpeningFiles.erase(sConfigFilePath);
    mtx.unlock();
    return -1;
  }

  mtx.lock();
  sOpeningFiles.erase(sConfigFilePath);
  mtx.unlock();

  return iResult;
}
#endif

#ifdef _WIN32
int PmonController::init() {
  int iResult = 0;

  logEvent("Initializing controller", LOG_TYPE_INFO);

  HKEY hkTop = CONFIG_REG_KEY_TOP_HKEY;

  iResult = RegCreateKeyExA(hkTop, CONFIG_REG_KEY.c_str(), 0, NULL, 0,
                            KEY_READ | KEY_WRITE, NULL, &hkConfig, NULL);
  if (iResult) {
    logEvent(std::format("RegCreateKeyExA failed {} for key {}\n", iResult,
                         CONFIG_REG_KEY),
             LOG_TYPE_ERROR);
    return iResult;
  }

  bStop = 0;

  std::vector<std::string> vsConfigSubKeys;
  iResult = getAllSubkeys(hkConfig, vsConfigSubKeys);
  if (iResult) {
    logEvent(std::format("getAllSubkeys failed {} for key {}\n", iResult,
                         CONFIG_REG_KEY),
             LOG_TYPE_ERROR);
    return iResult;
  }

  std::map<std::string, std::vector<DWORD> > mProcNamesToPids;
  iResult = getAllProcessNamesAndPids(mProcNamesToPids);
  if (iResult) {
    logEvent(std::format("getAllProcessNamesAndPids failed {} for key {}\n",
                         iResult, CONFIG_REG_KEY),
             LOG_TYPE_ERROR);
    return iResult;
  }

  for (auto sConfigFilePath : vsConfigSubKeys) {
    auto sProcName = sConfigFilePath;
    std::vector<DWORD> viPids = mProcNamesToPids[sProcName];

    for (auto iPid : viPids) {
      mProcsByPid[iPid] = new Process(iPid);
      mProcsByPid[iPid]->updateInfo();
      // std::cout << mProcsByPid[iPid]->toString() << "\n";
    }
  }

  logEvent("Successfully initialized process objects", LOG_TYPE_INFO);

  while (!qEvents.empty()) {
    qEvents.pop();
  }

  hNpSendEvent =
      CreateNamedPipeA(NP_NAME_SEND_EVENT.c_str(), PIPE_ACCESS_DUPLEX,
                       PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_NOWAIT,
                       PIPE_UNLIMITED_INSTANCES, NP_MAX_BUFFER_SIZE,
                       NP_MAX_BUFFER_SIZE, 0, NULL);
  if (hNpSendEvent == INVALID_HANDLE_VALUE) {
    iResult = GetLastError();
    logEvent(std::format("CreateNamedPipeA failed {} for {}\n", iResult,
                         NP_NAME_SEND_EVENT),
             LOG_TYPE_ERROR);
    return iResult;
  }

  hNpRecvConfigChange =
      CreateNamedPipeA(NP_NAME_CONFIG_CHANGE.c_str(), PIPE_ACCESS_DUPLEX,
                       PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_NOWAIT,
                       PIPE_UNLIMITED_INSTANCES, NP_MAX_BUFFER_SIZE,
                       NP_MAX_BUFFER_SIZE, 0, NULL);
  if (hNpSendEvent == INVALID_HANDLE_VALUE) {
    iResult = GetLastError();
    logEvent(std::format("CreateNamedPipeA failed {} for {}\n", iResult,
                         NP_NAME_CONFIG_CHANGE),
             LOG_TYPE_ERROR);
    return iResult;
  }

  logEvent("Successfully opened backend named pipes", LOG_TYPE_INFO);

  fMonitorProcessThread =
      std::async(&PmonController::monitorProcessesThreadMain, this);
  logEvent("Successfully started backend monitoring processes thread ",
           LOG_TYPE_INFO);

  fSendEventThread = std::async(&PmonController::sendEventThreadMain, this);
  logEvent("Successfully started event sending thread", LOG_TYPE_INFO);

  fHandleConfigChangeThread =
      std::async(&PmonController::handleConfigChangeThreadMain, this);
  logEvent("Successfully started configuration changes handling thread",
           LOG_TYPE_INFO);

  return 0;
}

int PmonController::mainThreadMain() {
  while (!bStop) {
    // system("clear");
    std::cout << ">";
    std::string sCmd;
    std::cin >> sCmd;
    if (sCmd == "stop") {
      bStop = 1;
    }
  }

  return 0;
}

int PmonController::clean() {
  bStop = 1;
  int iResult = 0;

  int iResult1 = fMonitorProcessThread.get();
  logEvent(std::format("Process monitoring thread terminated with status {}\n",
                       iResult1),
           LOG_TYPE_INFO);

  int iResult2 = fSendEventThread.get();
  logEvent(
      std::format("Communication thread terminated with status {}\n", iResult2),
      LOG_TYPE_INFO);

  int iResult3 = fHandleConfigChangeThread.get();
  logEvent(
      std::format(
          "Configuration change handling thread terminated with status {}\n",
          iResult3),
      LOG_TYPE_INFO);

  logEvent("Successfully terminated backend threads", LOG_TYPE_INFO);

  if (!CloseHandle(hNpSendEvent)) {
    iResult = GetLastError();
    logEvent(std::format("CloseHandle failed {} for {}\n", iResult,
                         NP_NAME_SEND_EVENT),
             LOG_TYPE_ERROR);
    return iResult;
  }

  if (!CloseHandle(hNpRecvConfigChange)) {
    iResult = GetLastError();
    logEvent(std::format("CloseHandle failed {} for {}\n", iResult,
                         NP_NAME_CONFIG_CHANGE),
             LOG_TYPE_ERROR);
    return iResult;
  }

  logEvent("Successfully closed backend named pipes", LOG_TYPE_INFO);

  for (auto p : mProcsByPid) {
    // std::cout << "About to delete p.second";
    delete p.second;
  }
  mProcsByPid.clear();

  logEvent("Successfully freed backend process objects", LOG_TYPE_INFO);

  iResult = RegCloseKey(hkConfig);
  if (iResult) {
    logEvent(std::format("RegCloseKey() failed {} for key {}\n", iResult,
                         CONFIG_REG_KEY),
             LOG_TYPE_ERROR);
    return iResult;
  }

  logEvent("Successfully shut down the backend controller", LOG_TYPE_INFO);

  return 0;
}

int PmonController::monitorProcessesThreadMain() {
  int iResult = 0;

  while (!bStop) {
    Sleep(3000);

    std::vector<std::string> vsConfigSubkeys;
    iResult = getAllSubkeys(hkConfig, vsConfigSubkeys);
    if (iResult) {
      logEvent(std::format("getAllSubkeys failed {} for key {}\n", iResult,
                           CONFIG_REG_KEY),
               LOG_TYPE_ERROR);
      return iResult;
    }

    std::map<std::string, std::vector<DWORD> > mProcNamesToPids;
    iResult = getAllProcessNamesAndPids(mProcNamesToPids);
    if (iResult) {
      logEvent(std::format("getAllProcessNamesAndPids failed {} for key {} \n",
                           iResult, CONFIG_REG_KEY),
               LOG_TYPE_ERROR);
      return iResult;
    }

    for (auto& sConfigSubkey : vsConfigSubkeys) {
      std::vector<DWORD> viPids = mProcNamesToPids[sConfigSubkey];
      if (viPids.empty()) {
        continue;
      }
      for (auto iPid : viPids) {
        if (mProcsByPid[iPid] == NULL) {
          mProcsByPid[iPid] = new Process(iPid);
        }
        mProcsByPid[iPid]->updateInfo();
        std::cout << mProcsByPid[iPid]->toString() << "\n";
      }

      Configuration cfg;
      iResult = getConfigFromKey(hkConfig, sConfigSubkey, cfg);
      if (iResult) {
        logEvent(std::format("getConfigFromKey failed {} for key {} \n",
                             iResult, CONFIG_REG_KEY + "\\" + sConfigSubkey),
                 LOG_TYPE_ERROR);
        return iResult;
      }
      std::cout << cfg.toString();

      for (auto iPid : viPids) {
        auto iViolationInfo =
            OverloadEvent::getViolationInfo(*mProcsByPid[iPid], cfg);
        // std::cout << iViolationInfo << "\n";
        if (iViolationInfo) {
          Event* evt = new OverloadEvent(*mProcsByPid[iPid], cfg);
          std::cout << evt->toString() << "\n";
          qEvents.push(evt);
          // delete evt;
        }
      }
    }
  }
  //// fHandleConfigChangeThread.get();
  return iResult;
}

int PmonController::sendEventThreadMain() {
  int iResult = 0;

  while (!bStop) {
    mtx.lock();
    if (qEvents.empty()) {
      mtx.unlock();
      continue;
    }
    mtx.unlock();

    auto pEvent = qEvents.front();

    BOOL bResult = FALSE;

    while (!bStop) {
      bResult = ConnectNamedPipe(hNpSendEvent, NULL);
      if (!bResult) {
        iResult = GetLastError();
        switch (iResult) {
          case ERROR_PIPE_LISTENING:
            logEvent("No clients for sending event. Retrying in 1 second.",
                     LOG_TYPE_WARNING);
            Sleep(1000);
            break;
          case ERROR_PIPE_CONNECTED:
            break;
          case ERROR_NO_DATA:
            if (!DisconnectNamedPipe(hNpSendEvent)) {
              iResult = GetLastError();
              logEvent(std::format("DisconnectNamedPipe failed {} for {} \n",
                                   iResult, NP_NAME_SEND_EVENT),
                       LOG_TYPE_ERROR);
              return iResult;
            }
            break;
          default:
            logEvent(std::format("ConnectNamedPipe failed {} for {} \n",
                                 iResult, NP_NAME_SEND_EVENT),
                     LOG_TYPE_ERROR);
            return iResult;
        }
        if (iResult == ERROR_PIPE_CONNECTED) {
          iResult = 0;
          break;
        }
      }
    }

    iResult = 0;
    bResult = WriteFile(hNpSendEvent, pEvent->toString().c_str(),
                        (DWORD)pEvent->toString().size(), NULL, NULL);
    if (!bResult) {
      iResult = GetLastError();
      logEvent(std::format("WriteFile failed {} for {}. Retrying in 1s.\n",
                           iResult, NP_NAME_SEND_EVENT),
               LOG_TYPE_ERROR);
      Sleep(1000);
    } else {
      delete pEvent;
      qEvents.pop();
    }
  }
  //// fHandleConfigChangeThread.get();
  //// std::cout << "Run here\n";
  return iResult;
}

int PmonController::handleConfigChangeThreadMain() {
  int iResult = 0;

  while (!bStop) {
    //  mq_attr mqa;
    //  mq_getattr(mqdRecvConfigChange, &mqa);
    BOOL bResult = FALSE;

    while (!bStop) {
      bResult = ConnectNamedPipe(hNpRecvConfigChange, NULL);
      if (!bResult) {
        iResult = GetLastError();
        switch (iResult) {
          case ERROR_PIPE_LISTENING:
            logEvent("No clients for sending event. Retrying in 1 second.",
                     LOG_TYPE_WARNING);
            Sleep(1000);
            break;
          case ERROR_PIPE_CONNECTED:
            break;
          case ERROR_NO_DATA:
            if (!DisconnectNamedPipe(hNpRecvConfigChange)) {
              iResult = GetLastError();
              logEvent(std::format("DisconnectNamedPipe failed {} for {} \n",
                                   iResult, NP_NAME_CONFIG_CHANGE),
                       LOG_TYPE_ERROR);
              return iResult;
            }
            break;
          default:
            logEvent(std::format("ConnectNamedPipe failed {} for {} \n",
                                 iResult, NP_NAME_CONFIG_CHANGE),
                     LOG_TYPE_ERROR);
            return iResult;
        }
        if (iResult == ERROR_PIPE_CONNECTED) {
          iResult = 0;
          break;
        }
      }
    }

    CHAR pcRecvBuffer[NP_MAX_BUFFER_SIZE] = "";
    DWORD uiNumberOfBytesRead = 0;
    MINI_CONFIGURATION mcfg;
    ZeroMemory(&mcfg, sizeof(MINI_CONFIGURATION));
    bResult = 0;

    // while (!bStop) {
    bResult = ReadFile(hNpRecvConfigChange, pcRecvBuffer, sizeof(pcRecvBuffer),
                       &uiNumberOfBytesRead, NULL);
    if (!bResult) {
      iResult = GetLastError();
      /*if (iResult != ERROR_NO_DATA) {*/
      logEvent(std::format("ReadFile failed {} for {}. Retrying in 1s.\n",
                           iResult, NP_NAME_CONFIG_CHANGE),
               LOG_TYPE_ERROR);
      Sleep(1000);
      /*} else {
        break;
      }*/

    } else {
      if (uiNumberOfBytesRead == 0) {
        Sleep(1000);
      } else {
        // CopyMemory(&mcfg, pcRecvBuffer, sizeof(MINI_CONFIGURATION));
        memcpy_s(&mcfg, sizeof(mcfg), pcRecvBuffer, sizeof(mcfg));
        Configuration cfg(mcfg);
        if (cfg.getName().front() == '!') {
          iResult = handleDeleteConfig(cfg);
          if (iResult) {
            logEvent(std::format("handleDeleteConfig() failed {}.\n", iResult),
                     LOG_TYPE_ERROR);
          }
        } else {
          iResult = handleCreateConfig(cfg);
          if (iResult) {
            logEvent(std::format("handleCreateConfig() failed {}.\n", iResult),
                     LOG_TYPE_ERROR);
          }
        }
        std::cout << "Config created: " << cfg.toString() << "\n";
        // break;
      }
      //// fSendEventThread.get();
      //// std::cout << iResult;
    }
    //}

    /*while (!bStop) {
      bResult = WriteFile(hNpRecvConfigChange, "$", 1, NULL, NULL);
      if (!bResult) {
        iResult = GetLastError();
        logEvent(std::format("WriteFile failed {} for {}. Retrying in 1s.\n",
                             iResult, NP_NAME_CONFIG_CHANGE),
                 LOG_TYPE_ERROR);
        Sleep(1000);
      } else {
        break;
      }
    }*/
  }
  return iResult;
}

int PmonController::handleCreateConfig(const Configuration& cfg) {
  int iResult = 0;

  // std::string sConfigKeyPath =
  //     CONFIG_REG_KEY + "\\" + cfg.getName() + ".cfg";

  HKEY hkConfigSubkey;
  iResult = RegCreateKeyA(hkConfig, cfg.getName().c_str(), &hkConfigSubkey);
  if (iResult) {
    logEvent(std::format("RegCreateKeyA failed {} for key {}", iResult,
                         CONFIG_REG_KEY),
             LOG_TYPE_ERROR);
    return iResult;
  }

  std::string sCpu = std::format("{:.2f}", cfg.getCpu());
  iResult = RegSetValueExA(hkConfigSubkey, "cpu", 0, REG_SZ,
                           (LPBYTE)sCpu.c_str(), (DWORD)sCpu.length());
  if (iResult) {
    logEvent(std::format("RegSetValueExA failed {} for key {}, value \"{}\"",
                         iResult, CONFIG_REG_KEY + "\\" + cfg.getName(), "cpu"),
             LOG_TYPE_ERROR);
    return iResult;
  }

  std::string sRam = std::format("{:.2f}", cfg.getRam());
  iResult = RegSetValueExA(hkConfigSubkey, "ram", 0, REG_SZ,
                           (LPBYTE)sRam.c_str(), (DWORD)sRam.length());
  if (iResult) {
    logEvent(std::format("RegSetValueExA failed {} for key {}, value \"{}\"",
                         iResult, CONFIG_REG_KEY + "\\" + cfg.getName(), "ram"),
             LOG_TYPE_ERROR);
    return iResult;
  }

  std::string sDisk = std::format("{:.2f}", cfg.getDisk());
  iResult = RegSetValueExA(hkConfigSubkey, "disk", 0, REG_SZ,
                           (LPBYTE)sDisk.c_str(), (DWORD)sDisk.length());
  if (iResult) {
    logEvent(
        std::format("RegSetValueExA failed {} for key {}, value \"{}\"",
                    iResult, CONFIG_REG_KEY + "\\" + cfg.getName(), "disk"),
        LOG_TYPE_ERROR);
    return iResult;
  }

  std::string sNet = std::format("{:.2f}", cfg.getNet());
  iResult = RegSetValueExA(hkConfigSubkey, "net", 0, REG_SZ,
                           (LPBYTE)sNet.c_str(), (DWORD)sNet.length());
  if (iResult) {
    logEvent(std::format("RegSetValueExA failed {} for key {}, value \"{}\"",
                         iResult, CONFIG_REG_KEY + "\\" + cfg.getName(), "net"),
             LOG_TYPE_ERROR);
    return iResult;
  }

  iResult = RegCloseKey(hkConfigSubkey);
  if (iResult) {
    logEvent(std::format("RegCloseKey() failed {} for key {}\n", iResult,
                         CONFIG_REG_KEY + "\\" + cfg.getName()),
             LOG_TYPE_ERROR);
    return iResult;
  }

  return iResult;
}

int PmonController::handleDeleteConfig(const Configuration& cfg) {
  int iResult = 0;

  std::string sConfigName = cfg.getName().substr(1);

  iResult = RegDeleteKeyExA(hkConfig, sConfigName.c_str(),
                            KEY_WOW64_32KEY | KEY_WOW64_64KEY, 0);
  if (iResult) {
    logEvent(std::format("RegDeleteKeyExA() failed {} for key {}\n", iResult,
                         CONFIG_REG_KEY + "\\" + cfg.getName()),
             LOG_TYPE_ERROR);
    return iResult;
  }

  return iResult;
}

int PmonController::getAllSubkeys(const HKEY& hkey,
                                  std::vector<std::string>& vsSubkeys) {
  int iResult = 0;

  DWORD uiNumberOfSubkeys = 0;
  DWORD uiMaxSubkeyNameLength = 0;
  iResult = RegQueryInfoKeyA(hkey, NULL, NULL, NULL, &uiNumberOfSubkeys,
                             &uiMaxSubkeyNameLength, NULL, NULL, NULL, NULL,
                             NULL, NULL);
  if (iResult) {
    logEvent(std::format("RegQueryInfoKeyA failed {} for key {}\n", iResult,
                         CONFIG_REG_KEY),
             LOG_TYPE_ERROR);
    return iResult;
  }
  for (DWORD i = 0; i < uiNumberOfSubkeys; i++) {
    LPSTR pcSubkeyName = new CHAR[uiMaxSubkeyNameLength + 1];
    DWORD uiSubkeyNameLength = uiMaxSubkeyNameLength + 1;
    ZeroMemory(pcSubkeyName, uiMaxSubkeyNameLength);

    iResult = RegEnumKeyExA(hkey, i, pcSubkeyName, &uiSubkeyNameLength, NULL,
                            NULL, NULL, NULL);
    if (iResult) {
      logEvent(std::format("RegQueryInfoKeyA failed {} for key {}\n", iResult,
                           CONFIG_REG_KEY),
               LOG_TYPE_ERROR);
      delete[] pcSubkeyName;
      return iResult;
    }
    vsSubkeys.push_back(pcSubkeyName);

    delete[] pcSubkeyName;
  }

  return iResult;
}

int PmonController::getAllProcessNamesAndPids(
    std::map<std::string, std::vector<DWORD> >& mProcNamesToPids) {
  int iResult = 0;

  HANDLE hSnapshot;
  hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  PROCESSENTRY32 entry;
  ZeroMemory(&entry, sizeof(PROCESSENTRY32));
  entry.dwSize = sizeof(PROCESSENTRY32);

  if (Process32First(hSnapshot, &entry)) {
    while (1) {
      if (Process32Next(hSnapshot, &entry)) {
        std::wstring wstr = entry.szExeFile;
        std::string sFileName = Utils::wstringToString(wstr);
        DWORD uiPid = entry.th32ProcessID;
        mProcNamesToPids[sFileName].push_back(uiPid);
      } else {
        iResult = GetLastError();
        if (iResult != ERROR_NO_MORE_FILES) {
          logEvent(std::format("Process32Next failed {}\n", iResult),
                   LOG_TYPE_ERROR);
          return iResult;
        } else {
          iResult = 0;
        }
        break;
      }
    }
  } else {
    iResult = GetLastError();
    if (iResult != ERROR_NO_MORE_FILES) {
      logEvent(std::format("Process32First failed {}\n", iResult),
               LOG_TYPE_ERROR);
      return iResult;
    } else {
      iResult = 0;
    }
  }

  if (!CloseHandle(hSnapshot)) {
    iResult = GetLastError();
    logEvent(std::format("CloseHandle failed {} for hSnapshot\n", iResult),
             LOG_TYPE_ERROR);
    return iResult;
  }

  return iResult;
}

int PmonController::getConfigFromKey(const HKEY& hkey,
                                     const std::string& sSubkeyPath,
                                     Configuration& cfg) {
  int iResult = 0;

  CHAR pcCpu[16] = "";
  CHAR pcRam[16] = "";
  CHAR pcDisk[16] = "";
  CHAR pcNet[16] = "";
  DWORD uiDataLen = 16;
  LPSTR pcStop;

  uiDataLen = 16;
  iResult = RegGetValueA(hkey, sSubkeyPath.c_str(), "cpu", RRF_RT_REG_SZ, NULL,
                         pcCpu, &uiDataLen);
  if (iResult) {
    logEvent(std::format("RegGetValueA failed {} for key {}, value \"cpu\"\n",
                         iResult, CONFIG_REG_KEY + "\\" + sSubkeyPath),
             LOG_TYPE_ERROR);
    return iResult;
  }
  DOUBLE dCpu = strtod(pcCpu, &pcStop);

  uiDataLen = 16;
  iResult = RegGetValueA(hkey, sSubkeyPath.c_str(), "ram", RRF_RT_REG_SZ, NULL,
                         pcRam, &uiDataLen);
  if (iResult) {
    logEvent(std::format("RegGetValueA failed {} for key {}, value \"ram\"\n",
                         iResult, CONFIG_REG_KEY + "\\" + sSubkeyPath),
             LOG_TYPE_ERROR);
    return iResult;
  }
  DOUBLE dRam = strtod(pcRam, &pcStop);

  uiDataLen = 16;
  iResult = RegGetValueA(hkey, sSubkeyPath.c_str(), "disk", RRF_RT_REG_SZ, NULL,
                         pcDisk, &uiDataLen);
  if (iResult) {
    logEvent(std::format("RegGetValueA failed {} for key {}, value \"disk\"\n",
                         iResult, CONFIG_REG_KEY + "\\" + sSubkeyPath),
             LOG_TYPE_ERROR);
    return iResult;
  }
  DOUBLE dDisk = strtod(pcDisk, &pcStop);

  uiDataLen = 16;
  iResult = RegGetValueA(hkey, sSubkeyPath.c_str(), "net", RRF_RT_REG_SZ, NULL,
                         pcNet, &uiDataLen);
  if (iResult) {
    logEvent(std::format("RegGetValueA failed {} for key {}, value \"net\"\n",
                         iResult, CONFIG_REG_KEY + "\\" + sSubkeyPath),
             LOG_TYPE_ERROR);
    return iResult;
  }
  DOUBLE dNet = strtod(pcNet, &pcStop);

  cfg = Configuration(sSubkeyPath, dCpu, dRam, dDisk, dNet);

  return iResult;
}

#endif

PmonController* PmonController::pInstance = nullptr;