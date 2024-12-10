#ifndef PROCESS_HPP
#define PROCESS_HPP

#ifdef _WIN32
#include <Windows.h>
#include <psapi.h>
#endif

#include <format>
#include <fstream>
#include <iostream>

#include "utils/EventLogger.hpp"

class Process {
 public:
  Process(int iPid);

  ~Process();

  int getPid() const;

  double getCpu() const;

  double getRam() const;

  double getDisk() const;

  double getNet() const;

  int updateInfo();

  std::string toString();

 private:
  int iPid;
  double dCpu;
  double dRam;
  double dDisk;
  double dNet;
  unsigned long long ullLastCpuTime;
  unsigned long long ullLastSysCpuTime;
  unsigned long long ullLastRchar;
  unsigned long long ullLastWchar;
  time_t tLastUpdated;

  int updateCpu();

  int updateRam();

  int updateDisk();

  int updateNet();
};

#endif