#include "EventLogger.hpp"

EventLogger* EventLogger::getInstance() {
  if (pInstance == nullptr) {
    pInstance = new EventLogger();
  }
  return pInstance;
}

int EventLogger::setLogDir(std::string sNewLogDir) {
  sLogDir = sNewLogDir;
  return 0;
}

int EventLogger::logEvent(std::string sEvent, LOG_TYPE logType) {
  time_t tNow = time(0);
  tm* ptm = localtime(&tNow);

  int iYear = ptm->tm_year + 1900;
  int iMonth = ptm->tm_mon + 1;
  int iDay = ptm->tm_mday;
  int iHour = ptm->tm_hour;
  int iMinute = ptm->tm_min;
  int iSecond = ptm->tm_sec;

  std::string sLogType = "";
  switch (logType) {
    case LOG_TYPE_INFO:
      sLogType = "INFO";
      break;
    case LOG_TYPE_WARNING:
      sLogType = "WARNING";
      break;
    case LOG_TYPE_ERROR:
      sLogType = "ERROR";
      break;
    default:
      break;
  }

  fo.open(sLogDir, std::ios::app);
  fo << std::format("[{}/{}/{} {:02d}:{:02d}:{:02d}][{}] {}\n", iDay, iMonth,
                    iYear, iHour, iMinute, iSecond, sLogType, sEvent);
  fo.close();
  return 0;
}

EventLogger* EventLogger::pInstance = nullptr;

EventLogger::EventLogger() {
  system("mkdir -p /tmp/rat/");
  sLogDir = DEFAUT_LOG_DIR;
}

EventLogger::~EventLogger() {}

int setLogDir(std::string sNewLogDir) {
  return EventLogger::getInstance()->setLogDir(sNewLogDir);
}

int logEvent(std::string sEvent, LOG_TYPE logType = LOG_TYPE_INFO) {
  return EventLogger::getInstance()->logEvent(sEvent, logType);
}