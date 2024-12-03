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

PmonController::PmonController() {}

PmonController::~PmonController() {}

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

PmonController* PmonController::pInstance = nullptr;