#ifdef __linux__
#include <mqueue.h>
#include <unistd.h>
#endif
#ifdef _WIN32
#include <Windows.h>
#endif

#include <format>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <sstream>

#include "Configuration.hpp"
#include "utils/EventLogger.hpp"
#include "utils/FastSystem.hpp"

const std::string EVENT_DIR_PATH = "events/";
#ifdef __linux__
const std::string MQ_RECV_EVENT_NAME = "/mqpmonevt";
const std::string MQ_SEND_CONFIG_CHANGE_REQUEST_NAME = "/mqpmoncfg";
#endif
#ifdef _WIN32
const std::string NP_NAME_RECV_EVENT = "\\\\.\\pipe\\nppmonevt";
const std::string NP_NAME_SEND_CONFIG_CHANGE_REQUEST = "\\\\.\\pipe\\nppmoncfg";
const int NP_MAX_BUFFER_SIZE = 1024;
#endif

class PmonUIController {
 public:
  static PmonUIController* getInstance();

  int run();

 private:
  static PmonUIController* pInstance;
  std::future<int> fRequestChangeConfigThread;
  std::future<int> fHandleNotificationThread;
  std::queue<Configuration*> qConfigQueue;
  std::mutex mtx;
#ifdef __linux__
  mqd_t mqdRecvEvent;
  mqd_t mqdSendConfigChangeReq;
#endif
#ifdef _WIN32
  HANDLE hNpRecvEvent;
  HANDLE hNpSendConfigChangeReq;
#endif

  bool bStop;

  PmonUIController();
  ~PmonUIController();

  int init();

  int mainThreadMain();

  int clean();

  int requestChangeConfigThreadMain();

  int handleNotificationThreadMain();

  int requestCreateConfig(Configuration* pCfg);

  int requestDeleteConfig(const std::string& sProcName);
};