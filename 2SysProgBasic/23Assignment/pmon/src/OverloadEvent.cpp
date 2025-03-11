#include "pmon/OverloadEvent.hpp"

OverloadEvent::OverloadEvent() {
    pProc = NULL;
    pConf = NULL;
    sProcInfo = "";
    sConfInfo = "";
    overloadType = 0;
}

OverloadEvent::OverloadEvent(Process& proc, Configuration& conf) {
  pProc = &proc;
  pConf = &conf;
  sProcInfo = proc.toString();
  sConfInfo = conf.toString();
  overloadType = getViolationInfo(proc, conf);
}

OverloadEvent::~OverloadEvent() {}

std::string OverloadEvent::toString() {
  std::string sViolations = "";
  if ((overloadType >> 0) & 1) {
    sViolations += "\"cpu\", ";
  }
  if ((overloadType >> 1) & 1) {
    sViolations += "\"ram\", ";
  }
  if ((overloadType >> 2) & 1) {
    sViolations += "\"disk\", ";
  }
  if ((overloadType >> 3) & 1) {
    sViolations += "\"net\", ";
  }

  if (!sViolations.empty()) {
    sViolations.pop_back();
    sViolations.pop_back();
  }

  return std::format(
      "{{ \"id\": {}, \"process\": {}, \"configuration\": {}, \"violations\": "
      "[ {} ] }}",
      getId(), sProcInfo, sConfInfo, sViolations);
}

int OverloadEvent::getViolationInfo(const Process& proc,
                                    const Configuration& conf) {
  int iRes = OVERLOAD_EVENT_VIOLATION_TYPE_NONE;
  if (proc.getCpu() >= conf.getCpu()) {
    iRes |= OVERLOAD_EVENT_VIOLATION_TYPE_CPU;
  }
  if (proc.getRam() >= conf.getRam()) {
    iRes |= OVERLOAD_EVENT_VIOLATION_TYPE_RAM;
  }
  if (proc.getDisk() >= conf.getDisk()) {
    iRes |= OVERLOAD_EVENT_VIOLATION_TYPE_DISK;
  }
  if (proc.getNet() >= conf.getNet()) {
    iRes |= OVERLOAD_EVENT_VIOLATION_TYPE_NET;
  }
  return iRes;
}