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
  /**
   * Singleton instance obtaining function
   */
  static PmonUIController* getInstance();

  /**
   * The main function of the frontend component, including a call to init(),
   * mainThreadMain() and clean()
   */
  int run();

 private:
  // Singleton instance
  static PmonUIController* pInstance;

  // Future object for the configuration change requesting thread, running
  // requestChangeConfigThreadMain()
  std::future<int> fRequestChangeConfigThread;

  // Future object for the notification handling thread, running
  // requestChangeConfigThreadMain()
  std::future<int> fHandleNotificationThread;

  // Configuration change request queue, used for sending requests
  std::queue<Configuration*> qConfigQueue;

  // Mutex object
  std::mutex mtx;
#ifdef __linux__
  // The message queue used for receiving events
  mqd_t mqdRecvEvent;

  // The message queue used for sending onfiguration change requests
  mqd_t mqdSendConfigChangeReq;
#endif
#ifdef _WIN32
  HANDLE hNpRecvEvent;
  HANDLE hNpSendConfigChangeReq;
#endif
  // The stop flag used for terminating threads
  bool bStop;

  /**
   * Default constructor
   */
  PmonUIController();

  /**
   * Default destructor
   */
  ~PmonUIController();

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
   * The main thread used for controling features.
   *
   * Available commands are:
   * - config new: Start an interactive session to request creating new
   * configurations
   * - config del: Start an interactive session to request deleting new
   * configurations
   * - events: View the event saving location
   * - quit: Quit the program
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
  int requestChangeConfigThreadMain();

  /**
   * Main function of the thread used for events notification.
   *
   * In this thread, messages are exchanged through a message queue (on Linux)
   * or a named pipe (on Windows). The IPC methods use nonblocking IO, and will
   * automatically retry when sending operation is not successful. Those
   * messages are saved on a queue object on heap memory.
   *
   */
  int handleNotificationThreadMain();

  /**
   * Create a config creating request
   *
   * @param pCfg Pointer to the configuration object
   *
   */
  int requestCreateConfig(Configuration* pCfg);

  /**
   * Create a config deleting request
   *
   * @param sProcName Name of the process affilated with the configuration  
   *
   */
  int requestDeleteConfig(const std::string& sProcName);
};