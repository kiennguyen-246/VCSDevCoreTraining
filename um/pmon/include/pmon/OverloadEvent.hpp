#ifndef OVERLOAD_EVENT_HPP
#define OVERLOAD_EVENT_HPP

#include <iostream>

#include "Configuration.hpp"
#include "Event.hpp"
#include "Process.hpp"

const int OVERLOAD_EVENT_VIOLATION_TYPE_NONE = 0;
const int OVERLOAD_EVENT_VIOLATION_TYPE_CPU = 1 << 0;
const int OVERLOAD_EVENT_VIOLATION_TYPE_RAM = 1 << 1;
const int OVERLOAD_EVENT_VIOLATION_TYPE_DISK = 1 << 2;
const int OVERLOAD_EVENT_VIOLATION_TYPE_NET = 1 << 3;

class OverloadEvent : public Event {
 public:
  OverloadEvent();

  OverloadEvent(Process& proc, Configuration& conf);

  ~OverloadEvent();

  std::string toString();

  static int getViolationInfo(const Process& proc, const Configuration& conf);

 private:
  Process* pProc;
  Configuration* pConf;
  std::string sProcInfo;
  std::string sConfInfo;
  int overloadType;
};

#endif