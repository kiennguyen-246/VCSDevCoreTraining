#include <chrono>
#include <ctime>
#include <fstream>

// #define setLogDir(dir) EventLogger::getInstance()->__setLogDir(dir)
// #define logEvent(event, type) \
//   EventLogger::getInstance()->__logEvent(event, type)

const std::string DEFAUT_LOG_DIR = "/tmp/logs/logfile.log";

typedef enum __LOG_TYPE {
  LOG_TYPE_INFO,
  LOG_TYPE_WARNING,
  LOG_TYPE_ERROR
} LOG_TYPE,
    *PLOG_TYPE;

class EventLogger {
 public:
  static EventLogger* getInstance();

  int setLogDir(std::string sNewLogDir);

  int logEvent(std::string sEvent, LOG_TYPE logType);

 private:
  static EventLogger* pInstance;

  std::string sLogDir;

  std::ofstream fo;

  EventLogger();
  ~EventLogger();
};

int setLogDir(std::string sNewLogDir);

int logEvent(std::string sEvent, LOG_TYPE logType);