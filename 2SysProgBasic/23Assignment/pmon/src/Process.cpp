#include "pmon/Process.hpp"

Process::Process(int __iPid) {
  iPid = __iPid;
  tLastUpdated = 0;
  dCpu = dRam = dDisk = dNet = 0;
  ullLastCpuTime = ullLastSysCpuTime = ullLastRchar = ullLastWchar = 0;
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
  std::string ret = std::format(
      "{{ \"pid\": {}, \"cpu\": {:.2f}, \"ram\": {:.2f}, \"disk\": {:.2f}, "
      "\"net\": {:.2f} }}",
      iPid, dCpu, dRam, dDisk, dNet);
  return ret;
}

#ifdef __linux__
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
            (tNow - tLastUpdated) / (1 << 20);
  }
  ullLastRchar = ullCurRchar;
  ullLastWchar = ullCurWchar;

  return 0;
}

int Process::updateNet() { return 0; }

#endif

#ifdef _WIN32
int Process::updateCpu() {
  int iResult = 0;
  unsigned long long ullCurCpuTime = 0;
  unsigned long long ullCurSysCpuTime = 0;

  HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, iPid);
  if (!hProc) {
    iResult = GetLastError();
    logEvent(std::format("OpenProcess() failed {} for process PID {}", iResult,
                         iPid),
             LOG_TYPE_ERROR);
    return iResult;
  }

  FILETIME ftProcCreation, ftProcExit, ftProcKernel, ftProcUser;
  if (!GetProcessTimes(hProc, &ftProcCreation, &ftProcExit, &ftProcKernel,
                       &ftProcUser)) {
    iResult = GetLastError();
    logEvent(std::format("GetProcessTimes() failed {} for process PID {}",
                         iResult, iPid),
             LOG_TYPE_ERROR);
    CloseHandle(hProc);
    return iResult;
  }

  if (!CloseHandle(hProc)) {
    iResult = GetLastError();
    logEvent(std::format("CloseHandle() failed {} for process PID {}", iResult,
                         iPid),
             LOG_TYPE_ERROR);
    return iResult;
  }

  FILETIME ftSysIdle, ftSysKernel, ftSysUser;
  if (!GetSystemTimes(&ftSysIdle, &ftSysKernel, &ftSysUser)) {
    iResult = GetLastError();
    std::cout << std::format("GetSystemTimes() failed {}", iResult);
    //CloseHandle(hProc);
    return iResult;
  }

  ullCurCpuTime =
      ULARGE_INTEGER{ftProcKernel.dwLowDateTime, ftProcKernel.dwHighDateTime}
          .QuadPart +
      ULARGE_INTEGER{ftProcUser.dwLowDateTime, ftProcUser.dwHighDateTime}
          .QuadPart;
  ullCurSysCpuTime =
      ULARGE_INTEGER{ftSysKernel.dwLowDateTime, ftSysKernel.dwHighDateTime}
          .QuadPart +
      ULARGE_INTEGER{ftSysUser.dwLowDateTime, ftSysUser.dwHighDateTime}
          .QuadPart;

  if (tLastUpdated) {
    dCpu = (double)(ullCurCpuTime - ullLastCpuTime) /
           (ullCurSysCpuTime - ullLastSysCpuTime) * 100;
  }

  ullLastCpuTime = ullCurCpuTime;
  ullLastSysCpuTime = ullCurSysCpuTime;

  return iResult;
}

int Process::updateRam() {
  int iResult = 0;

  HANDLE hProc =
      OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, iPid);
  if (!hProc) {
    iResult = GetLastError();
    logEvent(std::format("OpenProcess() failed {} for process PID {}", iResult,
                         iPid),
             LOG_TYPE_ERROR);
    return iResult;
  }

  PROCESS_MEMORY_COUNTERS pmc;
  if (!GetProcessMemoryInfo(hProc, &pmc, sizeof(PROCESS_MEMORY_COUNTERS))) {
    iResult = GetLastError();
    logEvent(std::format("GetProcessMemoryInfo() failed {} for process PID {}",
                         iResult, iPid),
             LOG_TYPE_ERROR);
    CloseHandle(hProc);
    return iResult;
  }

  if (!CloseHandle(hProc)) {
    iResult = GetLastError();
    logEvent(std::format("CloseHandle() failed {} for process PID {}", iResult,
                         iPid),
             LOG_TYPE_ERROR);
    return iResult;
  }

  dRam = (double)pmc.WorkingSetSize / (1 << 20);

  return iResult;
}

int Process::updateDisk() {
  int iResult = 0;

  unsigned long long ullCurRchar = 0;
  unsigned long long ullCurWchar = 0;

  HANDLE hProc =
      OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, iPid);
  if (!hProc) {
    iResult = GetLastError();
    logEvent(std::format("OpenProcess() failed {} for process PID {}", iResult,
                         iPid),
             LOG_TYPE_ERROR);
    return iResult;
  }

  IO_COUNTERS ioc;
  if (!GetProcessIoCounters(hProc, &ioc)) {
    iResult = GetLastError();
    logEvent(std::format("OpenProcess() failed {} for process PID {}", iResult,
                         iPid),
             LOG_TYPE_ERROR);
    return iResult;
  }

  if (!CloseHandle(hProc)) {
    iResult = GetLastError();
    logEvent(std::format("CloseHandle() failed {} for process PID {}", iResult,
                         iPid),
             LOG_TYPE_ERROR);
    return iResult;
  }

  ullCurRchar = ioc.ReadTransferCount;
  ullCurWchar = ioc.WriteTransferCount;

  time_t tNow = time(NULL);
  if (tLastUpdated) {
    dDisk = (double)(ullCurRchar + ullCurWchar - ullLastRchar - ullLastWchar) /
            (tNow - tLastUpdated) / (1 << 20);
  }
  ullLastRchar = ullCurRchar;
  ullLastWchar = ullCurWchar;

  return 0;
}

int Process::updateNet() { return 0; }

#endif