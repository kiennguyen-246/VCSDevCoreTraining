#ifdef __linux__
#include <mqueue.h>
#include <sys/wait.h>
#endif

#include <cstring>
#include <fstream>
#include <future>
#include <iostream>
#include <map>
#include <set>
#include <queue>
#include <sstream>
#include <vector>

#include "Configuration.hpp"
#include "OverloadEvent.hpp"
#include "Process.hpp"
#include "utils/FastSystem.hpp"
#include "utils/Utils.hpp"

const std::string CONFIG_DIR_PATH = "~/pmon/";
const std::string MQ_SEND_EVENT_NAME = "/mqpmonevt";
const std::string MQ_RECV_CONFIG_CHANGE_NAME = "/mqpmoncfg";

class PmonController {
 public:
  static PmonController* getInstance();

  int run();

 private:
  static PmonController* pInstance;
  std::map<int, Process*> mProcsByPid;
  std::future<int> fMonitorProcessThread;
  std::future<int> fSendEventThread;
  std::future<int> fHandleConfigChangeThread;
  std::mutex mtx;
  std::queue<Event*> qEvents;
  std::set<std::string> sOpeningFiles;
#ifdef __linux__
  mqd_t mqdSendEvent;
  mqd_t mqdRecvConfigChange;
#endif
  bool bStop;

  PmonController();

  ~PmonController();

  int init();

  int mainThreadMain();

  int clean();

  int monitorProcessesThreadMain();

  int sendEventThreadMain();

  int handleConfigChangeThreadMain();

  int handleCreateConfig(const Configuration& pCfg);

  int handleDeleteConfig(const Configuration& cfg);

  int parseConfigFile(const std::string sConfigFilePath, Configuration& cfg);
};