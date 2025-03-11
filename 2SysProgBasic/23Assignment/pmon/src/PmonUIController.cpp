#include "pmonui/PmonUIController.hpp"

PmonUIController* PmonUIController::getInstance() {
  if (pInstance == NULL) {
    pInstance = new PmonUIController();
  }
  return pInstance;
}

int PmonUIController::run() {
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

PmonUIController::PmonUIController() {
#if __linux__

#endif

#if _WIN32
  hNpRecvEvent = INVALID_HANDLE_VALUE;
  hNpSendConfigChangeReq = INVALID_HANDLE_VALUE;
#endif

  bStop = false;
}

PmonUIController::~PmonUIController() {}

int PmonUIController::mainThreadMain() {
  int iResult = 0;

  while (!bStop) {
    std::cout << "pmon> ";
    std::cout.flush();
    std::string sCmd;
    getline(std::cin, sCmd);
    std::istringstream issCmd(sCmd);
    std::string sOptions;
    issCmd >> sCmd >> sOptions;
    if (sCmd == "evt") {
      std::string sCwd;
#ifdef __linux__
      iResult = FastSystem::system("pwd", sCwd);
#endif

#ifdef _WIN32
      CHAR pcCwd[MAX_PATH] = "";
      iResult = GetCurrentDirectoryA(MAX_PATH, pcCwd);
      sCwd = pcCwd;
#endif
      std::cout << sCwd + EVENT_DIR_PATH << "\n";
    }
    if (sCmd == "quit") {
      bStop = 1;
    }
    if (sCmd == "config") {
      if (sOptions == "new") {
        std::string sProcName;
        double dCpu;
        double dRam;
        double dDisk;
        double dNet;
        std::cout << "Enter Process Name: ";
        std::cin >> sProcName;
        std::cout << "Enter CPU Utilization Limit (%): ";
        std::cin >> dCpu;
        std::cout << "Enter RAM Utilization Limit (MB): ";
        std::cin >> dRam;
        std::cout << "Enter Disk Utilization Limit (MBps): ";
        std::cin >> dDisk;
        std::cout << "Enter Network Utilization Limit (KBps): ";
        std::cin >> dNet;
        auto pCfg = new Configuration(sProcName, dCpu, dRam, dDisk, dNet);
        if (!requestCreateConfig(pCfg)) {
          std::cout << "New configuration request sent\n";
        } else {
          std::cout << "Failed to send configuration request\n";
        }
      }
      if (sOptions == "del") {
        std::string sProcName;
        std::cout << "Enter Process Name: ";
        std::cin >> sProcName;
        if (!requestDeleteConfig(sProcName)) {
          std::cout << "Delete configuration request sent\n";
        } else {
          std::cout << "Failed to send configuration request\n";
        }
      }
      getline(std::cin, sCmd);
    }
  }
  return 0;
}

int PmonUIController::requestCreateConfig(Configuration* pCfg) {
  qConfigQueue.push(pCfg);
  return 0;
}

int PmonUIController::requestDeleteConfig(const std::string& sProcName) {
  auto pCfg = new Configuration("!" + sProcName, 0, 0, 0, 0);
  qConfigQueue.push(pCfg);
  return 0;
}

#ifdef __linux__
int PmonUIController::init() {
  int iResult = 0;

  logEvent("Initializing UI controller", LOG_TYPE_INFO);

  iResult = FastSystem::mkdir(EVENT_DIR_PATH);
  if (iResult) {
    logEvent(std::format("FastSystem::mkdir failed {}\n", iResult),
             LOG_TYPE_ERROR);
    return iResult;
  }

  mq_attr mqa;
  memset(&mqa, 0, sizeof(mq_attr));
  mqa.mq_maxmsg = 50;
  mqa.mq_msgsize = 2048;
  mqdRecvEvent = mq_open(MQ_RECV_EVENT_NAME.c_str(),
                         O_CREAT | O_RDONLY | O_NONBLOCK, 0777, mqa);
  if (mqdRecvEvent == (mqd_t)-1) {
    logEvent(
        std::format("mq_open() failed {} for {}\n", errno, MQ_RECV_EVENT_NAME),
        LOG_TYPE_ERROR);
    return errno;
  }

  mqdSendConfigChangeReq = mq_open(MQ_SEND_CONFIG_CHANGE_REQUEST_NAME.c_str(),
                                   O_CREAT | O_WRONLY | O_NONBLOCK, 0777, mqa);
  if (mqdSendConfigChangeReq == (mqd_t)-1) {
    logEvent(std::format("mq_open() failed {} for {}\n", errno,
                         MQ_SEND_CONFIG_CHANGE_REQUEST_NAME),
             LOG_TYPE_ERROR);
    return errno;
  }

  fRequestChangeConfigThread =
      std::async(&PmonUIController::requestChangeConfigThreadMain, this);
  logEvent("Successfully started UI configuration handling thread ",
           LOG_TYPE_INFO);

  fHandleNotificationThread =
      std::async(&PmonUIController::handleNotificationThreadMain, this);
  logEvent("Successfully started event sending thread", LOG_TYPE_INFO);

  return iResult;
}

int PmonUIController::clean() {
  bStop = 1;
  int iResult = 0;

  int iResult1 = fHandleNotificationThread.get();
  logEvent(
      std::format("UI notification handling thread terminated with status {}\n",
                  iResult1),
      LOG_TYPE_INFO);

  int iResult2 = fRequestChangeConfigThread.get();
  logEvent(std::format("UI communication thread terminated with status {}\n",
                       iResult2),
           LOG_TYPE_INFO);

  logEvent("Successfully terminated UI threads", LOG_TYPE_INFO);

  iResult = mq_close(mqdRecvEvent);
  if (iResult) {
    logEvent(
        std::format("mq_close() failed {} for {}\n", errno, MQ_RECV_EVENT_NAME),
        LOG_TYPE_ERROR);
    return errno;
  }

  iResult = mq_close(mqdSendConfigChangeReq);
  if (iResult) {
    logEvent(std::format("mq_close() failed {} for {}\n", errno,
                         MQ_SEND_CONFIG_CHANGE_REQUEST_NAME),
             LOG_TYPE_ERROR);
    return errno;
  }

  logEvent("Successfully closed UI message queues", LOG_TYPE_INFO);

  logEvent("Successfully shut down the UI controller", LOG_TYPE_INFO);

  return iResult;
}

int PmonUIController::requestChangeConfigThreadMain() {
  int iResult = 0;

  while (!bStop) {
    mtx.lock();
    if (qConfigQueue.empty()) {
      mtx.unlock();
      continue;
    }
    mtx.unlock();

    Configuration* pCfg = qConfigQueue.front();
    MINI_CONFIGURATION mcfg = pCfg->minimize();
    char pcSendBuffer[2048];
    memset(pcSendBuffer, 0, sizeof(pcSendBuffer));
    memcpy(pcSendBuffer, &mcfg, sizeof(MINI_CONFIGURATION));
    // for (int i = 0; i < 64; i++) std::cout << (int)pcSendBuffer[i] << " ";
    // std::cout << "\n";
    iResult =
        mq_send(mqdSendConfigChangeReq, pcSendBuffer, sizeof(pcSendBuffer), 0);
    // std::cout << "\n";
    if (iResult) {
      int iErr = errno;
      if (iErr == EAGAIN) {
        logEvent(
            std::format("Sending message queue is full. Retrying in 1s.\n"),
            LOG_TYPE_WARNING);

      } else {
        logEvent(std::format("mq_send() failed {} for {}. Retrying in 1s.\n",
                             iErr, MQ_SEND_CONFIG_CHANGE_REQUEST_NAME),
                 LOG_TYPE_ERROR);
      }
      sleep(1);
    } else {
      delete pCfg;
      qConfigQueue.pop();
    }
  }

  return iResult;
}

int PmonUIController::handleNotificationThreadMain() {
  int iResult = 0;

  while (!bStop) {
    std::string sEvt;

    mq_attr mqa;
    mq_getattr(mqdRecvEvent, &mqa);

    char pcRecvBuffer[mqa.mq_msgsize];
    memset(pcRecvBuffer, 0, sizeof(pcRecvBuffer));
    iResult = mq_receive(mqdRecvEvent, pcRecvBuffer, mqa.mq_msgsize, 0);
    if (iResult == -1) {
      // std::cout << errno << "\n";
      int iErr = errno;
      if (iErr != EAGAIN) {
        logEvent(std::format("mq_receive() failed {} for {}. Retrying in 1s.\n",
                             iErr, MQ_RECV_EVENT_NAME),
                 LOG_TYPE_ERROR);
      }
      sleep(1);
    } else {
      // std::cout << "Receive ok\n";
      for (int i = 0; i < iResult; i++) {
        if (pcRecvBuffer[i] == 0) {
          break;
        }
        sEvt.push_back(pcRecvBuffer[i]);
      }
      // std::cout << sEvt << "\n";
      int iEventId = 0;
      std::istringstream issEvt(sEvt);
      std::string str;
      while (issEvt >> str) {
        if (str == "\"id\":") {
          issEvt >> str;
          iEventId = atoi(str.c_str());
        }
      }

      std::ofstream ofs(std::format("{}{}.json", EVENT_DIR_PATH, iEventId));
      if (ofs.is_open()) {
        ofs << sEvt;
        ofs.close();
      } else {
        logEvent(std::format("Failed to open event file {}{}.json.\n",
                             EVENT_DIR_PATH, iEventId),
                 LOG_TYPE_ERROR);
      }
    }
  }

  return iResult;
}
#endif

#ifdef _WIN32
int PmonUIController::init() {
  int iResult = 0;

  bStop = FALSE;

  logEvent("Initializing UI controller", LOG_TYPE_INFO);

  if (!CreateDirectoryA(".\\events", NULL)) {
    iResult = GetLastError();
    if (iResult != ERROR_ALREADY_EXISTS) {
      logEvent(
          std::format("CreateDirectory failed {} for \".\\events\"\n", iResult),
          LOG_TYPE_ERROR);
      return iResult;
    }
  }
  iResult = 0;

  fRequestChangeConfigThread =
      std::async(&PmonUIController::requestChangeConfigThreadMain, this);
  logEvent("Successfully started UI configuration handling thread ",
           LOG_TYPE_INFO);

  fHandleNotificationThread =
      std::async(&PmonUIController::handleNotificationThreadMain, this);
  logEvent("Successfully started event sending thread", LOG_TYPE_INFO);

  return iResult;
}

int PmonUIController::clean() {
  bStop = 1;
  int iResult = 0;

  int iResult1 = fHandleNotificationThread.get();
  logEvent(
      std::format("UI notification handling thread terminated with status {}\n",
                  iResult1),
      LOG_TYPE_INFO);

  int iResult2 = fRequestChangeConfigThread.get();
  logEvent(std::format("UI communication thread terminated with status {}\n",
                       iResult2),
           LOG_TYPE_INFO);

  logEvent("Successfully terminated UI threads", LOG_TYPE_INFO);

  logEvent("Successfully closed UI message queues", LOG_TYPE_INFO);

  logEvent("Successfully shut down the UI controller", LOG_TYPE_INFO);

  return iResult;
}

int PmonUIController::requestChangeConfigThreadMain() {
  int iResult = 0;

  while (!bStop) {
    hNpSendConfigChangeReq = CreateFileA(
        NP_NAME_SEND_CONFIG_CHANGE_REQUEST.c_str(),
        GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hNpSendConfigChangeReq == INVALID_HANDLE_VALUE) {
      iResult = GetLastError();
      if (iResult != ERROR_PIPE_BUSY) {
        logEvent(std::format("CreateFileA failed {} for {}\n", iResult,
                             NP_NAME_SEND_CONFIG_CHANGE_REQUEST),
                 LOG_TYPE_ERROR);
        return iResult;
      }
    } else {
      iResult = 0;
      break;
    }
  }

  while (!bStop) {
    mtx.lock();
    if (qConfigQueue.empty()) {
      mtx.unlock();
      continue;
    }
    mtx.unlock();

    Configuration* pCfg = qConfigQueue.front();
    MINI_CONFIGURATION mcfg = pCfg->minimize();
    CHAR pcSendBuffer[NP_MAX_BUFFER_SIZE] = "";
    CopyMemory(pcSendBuffer, &mcfg, sizeof(MINI_CONFIGURATION));

    BOOL bResult = FALSE;
    iResult = 0;
    // while (!bStop) {
    bResult = WriteFile(hNpSendConfigChangeReq, pcSendBuffer,
                        sizeof(MINI_CONFIGURATION), NULL, NULL);
    if (!bResult) {
      iResult = GetLastError();
      logEvent(std::format("WriteFile failed {} for {}. Retrying in 1s.\n",
                           iResult, NP_NAME_SEND_CONFIG_CHANGE_REQUEST),
               LOG_TYPE_ERROR);
      Sleep(1000);
    } else {
      delete pCfg;
      qConfigQueue.pop();
      // break;
    }

    // Sleep(30000);
    // }

    // logEvent("Sent OK", LOG_TYPE_INFO);

    // CHAR pcRecvBuffer[NP_MAX_BUFFER_SIZE] = "";
    // DWORD uiNumberOfBytesRead;
    // bResult = FALSE;
    // while (!bStop) {
    //   bResult = ReadFile(hNpSendConfigChangeReq, pcRecvBuffer,
    //                      sizeof(pcRecvBuffer), &uiNumberOfBytesRead, NULL);
    //   if (!bResult) {
    //     iResult = GetLastError();
    //     logEvent(std::format("ReadFile failed {} for {}. Retrying in 1s.\n",
    //                          iResult, NP_NAME_SEND_CONFIG_CHANGE_REQUEST),
    //              LOG_TYPE_ERROR);
    //     Sleep(1000);
    //   } else {
    //     if (uiNumberOfBytesRead == 1 && pcRecvBuffer[0] == '$') {
    //       break;
    //     }
    //     Sleep(1000);
    //     //// fSendEventThread.get();
    //     //// std::cout << iResult;
    //   }
    // }

    // while (!bStop) {
    //   CHAR pcPeekBuffer[NP_MAX_BUFFER_SIZE] = "";
    //   DWORD uiNumberOfBytesRead = 0;
    //   DWORD uiNumberOfBytesLeft = 0;
    //   if (!PeekNamedPipe(hNpSendConfigChangeReq, pcPeekBuffer,
    //   sizeof(pcPeekBuffer),
    //                      &uiNumberOfBytesRead,
    //                      &uiNumberOfBytesLeft, NULL)) {
    //     iResult = GetLastError();
    //     logEvent(std::format("PeekNamedPipe failed {} for {}", iResult,
    //                          NP_NAME_SEND_CONFIG_CHANGE_REQUEST),
    //              LOG_TYPE_ERROR);
    //     Sleep(1000);
    //   } else {
    //     std::cout << uiNumberOfBytesRead << " " << uiNumberOfBytesLeft;
    //     if (!uiNumberOfBytesRead && !uiNumberOfBytesLeft) {
    //       break;
    //     }
    //   }
    //   // std::cout << pcPeekBuffer << "\n";
    // }
  }

  if (!CloseHandle(hNpSendConfigChangeReq)) {
    iResult = GetLastError();
    logEvent(std::format("CloseHandle failed {} for {}\n", iResult,
                         NP_NAME_RECV_EVENT),
             LOG_TYPE_ERROR);
    return iResult;
  }

  return iResult;
}

int PmonUIController::handleNotificationThreadMain() {
  int iResult = 0;

  while (!bStop) {
    std::string sEvt;

    //  mq_attr mqa;
    //  mq_getattr(mqdRecvEvent, &mqa);

    while (!bStop) {
      hNpRecvEvent =
          CreateFileA(NP_NAME_RECV_EVENT.c_str(), GENERIC_READ | GENERIC_WRITE,
                      0, NULL, OPEN_EXISTING, 0, NULL);
      if (hNpRecvEvent == INVALID_HANDLE_VALUE) {
        iResult = GetLastError();
        if (iResult != ERROR_PIPE_BUSY) {
          logEvent(std::format("CreateFileA failed {} for {}\n", iResult,
                               NP_NAME_RECV_EVENT),
                   LOG_TYPE_ERROR);
          return iResult;
        }
      } else {
        iResult = 0;
        break;
      }
    }

    // while (!bStop) {
    CHAR pcRecvBuffer[NP_MAX_BUFFER_SIZE] = "";
    DWORD uiNumberOfBytesRead;
    bool bResult = FALSE;
    bResult = ReadFile(hNpRecvEvent, pcRecvBuffer, sizeof(pcRecvBuffer),
                       &uiNumberOfBytesRead, NULL);
    if (!bResult) {
      iResult = GetLastError();
      logEvent(std::format("ReadFile failed {} for {}.\n", iResult,
                           NP_NAME_RECV_EVENT),
               LOG_TYPE_ERROR);
    } else {
      if (uiNumberOfBytesRead == 0) {
        Sleep(1000);
      } else {
        std::string sEvt = pcRecvBuffer;
        int iEventId = 0;
        std::istringstream issEvt(sEvt);
        std::string str;
        while (issEvt >> str) {
          if (str == "\"id\":") {
            issEvt >> str;
            iEventId = atoi(str.c_str());
          }
        }
        std::ofstream ofs(std::format("{}{}.json", EVENT_DIR_PATH, iEventId));
        if (ofs.is_open()) {
          ofs << sEvt;
          ofs.close();
        } else {
          logEvent(std::format("Failed to open event file {}{}.json.\n",
                               EVENT_DIR_PATH, iEventId),
                   LOG_TYPE_ERROR);
        }
        break;
      }
    }
    //}
    if (!CloseHandle(hNpRecvEvent)) {
      iResult = GetLastError();
      logEvent(std::format("CloseHandle failed {} for {}\n", iResult,
                           NP_NAME_RECV_EVENT),
               LOG_TYPE_ERROR);
      return iResult;
    }
  }

  return iResult;
}
#endif  //  _WIN32

PmonUIController* PmonUIController::pInstance = nullptr;
