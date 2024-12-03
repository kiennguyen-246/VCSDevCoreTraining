#ifdef __linux__
#include <mqueue.h>
#include <unistd.h>
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
const std::string MQ_RECV_EVENT_NAME = "/mqpmonevt";
const std::string MQ_SEND_CONFIG_CHANGE_REQUEST_NAME = "/mqpmoncfg";

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