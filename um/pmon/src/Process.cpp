#include "pmon/Process.hpp"

Process::Process(int __iPid) {
  iPid = __iPid;
  tLastUpdated = 0;
  updateInfo();
}

Process::~Process() {}

int Process::getPid() const { return iPid; }

double Process::getCpu() const { return dCpu; }

double Process::getRam() const { return dRam; }

double Process::getDisk() const { return dDisk; }

double Process::getNet() const { return dNet; }

int Process::updateInfo() {
  int iResult = 0;

  iResult = updateCpu();
  iResult = updateRam();
  iResult = updateDisk();
  iResult = updateNet();

  tLastUpdated = time(NULL);

  return iResult;
}

std::string Process::toString() {
  return std::format(
      "{{ \"pid\": {}, \"cpu\": {:.2f}, \"ram\": {:.2f}, \"disk\": {:.2f}, "
      "\"net\": {:.2f} }}",
      iPid, dCpu, dRam, dDisk, dNet);
}

int Process::updateCpu() {
  unsigned long long ullCurCpuTime = 0;
  unsigned long long ullCurSysCpuTime = 0;

  std::ifstream ifs(std::format("/proc/{}/stat", iPid));
  if (ifs.is_open()) {
    std::string sVal;
    for (int i = 0; i < 14; i++) {
      ifs >> sVal;
    }
    ullCurCpuTime = atoll(sVal.c_str());
    ifs >> sVal;
    ullCurCpuTime += atoll(sVal.c_str());
    ifs.close();
  } else {
    logEvent(std::format("Cannot open /proc/{}/stat", iPid), LOG_TYPE_WARNING);
    return -1;
  }

  ifs.open("/proc/stat");
  if (ifs.is_open()) {
    std::string sVal;
    for (int i = 0; i < 14; i++) {
      ifs >> sVal;
      if (i != 3) {
        ullCurSysCpuTime += atoll(sVal.c_str());
      }
    }
    ifs.close();
  } else {
    logEvent(std::format("Cannot open /proc/stat"), LOG_TYPE_WARNING);
    return -1;
  }

  if (tLastUpdated) {
    dCpu = (double)(ullCurCpuTime - ullLastCpuTime) /
           (ullCurSysCpuTime - ullLastSysCpuTime) * 100;
  }

  ullLastCpuTime = ullCurCpuTime;
  ullLastSysCpuTime = ullCurSysCpuTime;

  return 0;
}

int Process::updateRam() {
  std::ifstream ifs(std::format("/proc/{}/stat", iPid));
  if (ifs.is_open()) {
    std::string sVal;
    for (int i = 0; i < 24; i++) {
      ifs >> sVal;
    }
    dRam = (double)atoll(sVal.c_str()) / 1000;
    ifs.close();
  } else {
    logEvent(std::format("Cannot open /proc/{}/stat", iPid), LOG_TYPE_WARNING);
    return -1;
  }

  return 0;
}

int Process::updateDisk() {
  unsigned long long ullCurRchar = 0;
  unsigned long long ullCurWchar = 0;

  std::ifstream ifs(std::format("/proc/{}/io", iPid));
  if (ifs.is_open()) {
    std::string sKey1, sRchar, sKey2, sWchar;
    ifs >> sKey1 >> sRchar >> sKey2 >> sWchar;
    ullCurRchar = atoll(sRchar.c_str());
    ullCurWchar = atoll(sWchar.c_str());
    ifs.close();
  } else {
    logEvent(std::format("Cannot open /proc/{}/io", iPid), LOG_TYPE_WARNING);
    return -1;
  }

  time_t tNow = time(NULL);
  if (tLastUpdated) {
    dDisk = (double)(ullCurRchar + ullCurWchar - ullLastRchar - ullLastWchar) /
            (tNow - tLastUpdated) / 1e6;
  }
  ullLastRchar = ullCurRchar;
  ullLastWchar = ullCurWchar;

  return 0;
}

int Process::updateNet() {
  dNet = 0;
  return 0;
}