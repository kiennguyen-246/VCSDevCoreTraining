#ifndef EVENT_LOGGER_HPP
#define EVENT_LOGGER_HPP

#ifdef _WIN32
#include <Windows.h>
#endif  // _WIN32


#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>

#include "utils/Utils.hpp"

// #define setLogDir(dir) EventLogger::getInstance()->__setLogDir(dir)
// #define logEvent(event, type) \
//   EventLogger::getInstance()->__logEvent(event, type)

#ifdef __linux__
const std::string DEFAUT_LOG_DIR_PATH = "~/logs";
const std::string DEFAUT_LOG_FILE_PATH = DEFAUT_LOG_DIR_PATH + "/logfile.log ";
#endif

#ifdef _WIN32
const std::string DEFAUT_LOG_DIR_PATH = "~\\AppData\\Local\\EventLogger";
const std::string DEFAUT_LOG_FILE_PATH = DEFAUT_LOG_DIR_PATH + "\\logfile.log ";
#endif

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

#endif