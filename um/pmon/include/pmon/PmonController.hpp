#ifdef __linux__
#include <mqueue.h>
#include <sys/wait.h>
#endif

#ifdef _WIN32
#include <TlHelp32.h>
#include <Windows.h>
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
  /**
   * Singleton instance obtaining function
   */
  static PmonController* getInstance();

  /**
   * The main function of the backend component, including a call to init(),
   * mainThreadMain() and clean()
   */
  int run();

 private:
  // Singleton instance
  static PmonController* pInstance;

  // A map structure used to manage process objects, categorized by their PIDs
  std::map<int, Process*> mProcsByPid;

  // Future object for the process monitoring thread, running
  // monitorProcessThreadMain()
  std::future<int> fMonitorProcessThread;

  // Future object for the event sending thread, running
  // handleSendEventThreadMain()
  std::future<int> fSendEventThread;

  // Future object for the configuration creation and deletion thread, running
  // handleConfigChangeThreadMain()
  std::future<int> fHandleConfigChangeThread;

  // Mutex object
  std::mutex mtx;

  // Event queue, used for sending events
  std::queue<Event*> qEvents;

  // A set object, used for monitoring opening files
  std::set<std::string> sOpeningFiles;
#ifdef __linux__
  // The message queue used for sending events
  mqd_t mqdSendEvent;

  // The message queue used for receiving onfiguration change requests
  mqd_t mqdRecvConfigChange;
#endif
#ifdef _WIN32
  HKEY hkConfig;
  HANDLE hNpSendEvent;
  HANDLE hNpRecvConfigChange;
#endif

  // The stop flag used for terminating threads
  bool bStop;

  /**
   * Default constructor
   */
  PmonController();

  /**
   * Default destructor
   */
  ~PmonController();

  /**
   * Main thread initialize function.
   *
   * In this function:
   * - Message queues/pipes are opened
   * - Threads are started
   *
   */
  int init();

  /**
   * Main thread main function.
   *
   * The main thread used for controling features, which include a single
   * command "stop" to stop the program.
   *
   */
  int mainThreadMain();

  /**
   * Main thread clean up function.
   *
   * In this function:
   * - Threads are terminated by setting bStop to 1
   * - Message queues/pipes are closed
   *
   */
  int clean();

  /**
   * Main function of the thread used for monitoring processes.
   *
   * In this thread, a set of running processes are monitored through Process
   * objects. They keep track of the processes specified by configuration
   * files/registry keys. When a configuration is removed, related Process are
   * also removed.
   *
   */
  int monitorProcessesThreadMain();

  /**
   * Main function of the thread used for events notification.
   *
   * In this thread, messages are exchanged through a message queue (on Linux)
   * or a named pipe (on Windows). The IPC methods use nonblocking IO, and will
   * automatically retry when sending operation is not successful. Those
   * messages are saved on a queue object on heap memory.
   *
   */
  int sendEventThreadMain();

  /**
   * Main function of the thread used for receiving configurtion change
   * requests.
   *
   * In this thread, messages are exchanged through a message queue (on Linux)
   * or a named pipe (on Windows). The IPC methods use nonblocking IO, and will
   * automatically retry when sending operation is not successful. Those
   * messages are saved on a queue object on heap memory.
   *
   * The requests after being deliered will be saved in a system file (on Linux)
   * or a registry key (on Windows).
   *
   */
  int handleConfigChangeThreadMain();

  /**
   * Save a configuration upon receiving a configuration change request.
   *
   * @param cfg The configuration object
   *
   */
  int handleCreateConfig(const Configuration& cfg);

  /**
   * Delete a configuration upon receiving a configuration change request.
   *
   * @param cfg The configuration object
   *
   */
  int handleDeleteConfig(const Configuration& cfg);

#ifdef __linux__
  /**
   * Parse the configuration file to get the configuration object saved within.
   *
   * @param sConfigFilePath Path to the configuration file
   * @param cfg The configuration object retrieved from the file
   *
   */
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