#ifdef __linux__
#include <mqueue.h>
#include <sys/wait.h>
#endif

#ifdef _WIN32
#include <Windows.h>
#include <TlHelp32.h>
#endif

#include <cstring>
#include <fstream>
#include <future>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <vector>

#include "Configuration.hpp"
#include "OverloadEvent.hpp"
#include "Process.hpp"
#include "utils/FastSystem.hpp"
#include "utils/Utils.hpp"

#ifdef __linux__
const std::string CONFIG_DIR_PATH = "~/pmon/";
const std::string MQ_SEND_EVENT_NAME = "/mqpmonevt";
const std::string MQ_RECV_CONFIG_CHANGE_NAME = "/mqpmoncfg";
#endif

#ifdef _WIN32
const HKEY CONFIG_REG_KEY_TOP_HKEY = HKEY_CURRENT_USER;
// const std::string CONFIG_REG_PARENT_KEY = "SOFTWARE";
const std::string CONFIG_REG_KEY = "SOFTWARE\\pmon";
const std::string NP_NAME_SEND_EVENT = "\\\\.\\pipe\\nppmonevt";
const std::string NP_NAME_CONFIG_CHANGE = "\\\\.\\pipe\\nppmoncfg";
const int NP_MAX_BUFFER_SIZE = 1024;
#endif

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
#ifdef _WIN32
  HKEY hkConfig;
  HANDLE hNpSendEvent;
  HANDLE hNpRecvConfigChange;
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

#ifdef __linux__
  int parseConfigFile(const std::string sConfigFilePath, Configuration& cfg);
#endif

#ifdef _WIN32
  int getAllSubkeys(const HKEY& hkey, std::vector<std::string>& vsSubkeys);

  int getAllProcessNamesAndPids(
      std::map<std::string, std::vector<DWORD> >& mProcNamesToPids);

  int getConfigFromKey(const HKEY& hkey, const std::string& sSubkeyPath,
                       Configuration& cfg);

#endif
};