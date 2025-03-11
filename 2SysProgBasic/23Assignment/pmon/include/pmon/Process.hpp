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
  /**
   * Construct a process object from a specified PID
   *
   * @param iPid The PID specified
   *
   */
  Process(int iPid);

  /**
   * Default destructor
   */
  ~Process();

  /**
   * Retrieve the PID of the process.
   */
  int getPid() const;

  /**
   * Retrieve the CPU ultilization saved.
   */
  double getCpu() const;

  /**
   * Retrieve the current memory ultilization of the process.
   */
  double getRam() const;

  /**
   * Retrieve the disk ultilization calculated from the last time
   * retrieved until present of the process.
   */
  double getDisk() const;

  /**
   * Retrieve the network ultilization calculated from the last time
   * retrieved until present of the process.
   */
  double getNet() const;

  /**
   * Update the values need recording.
   */
  int updateInfo();

  /**
   * Create a string to describe the current process object.
   */
  std::string toString();

 private:
  // The PID of the process
  int iPid;

  // The CPU ultilization calculated from the last time retrieved until present
  // of the process.
  double dCpu;

  // The current RAM ultilization of the process.
  double dRam;

  // The disk ultilization calculated from the last time retrieved until present
  // of the process.
  double dDisk;

  // The network ultilization calculated from the last time retrieved until
  // present of the process.
  double dNet;

  // The last CPU time of the process recorded by the object
  unsigned long long ullLastCpuTime;

  // The last system CPU time recorded by the object
  unsigned long long ullLastSysCpuTime;

  // The last number of bytes read from the disk recorded by the object
  unsigned long long ullLastRchar;

  // The last number of bytes written to the disk recorded by the object
  unsigned long long ullLastWchar;

  // The last timestamp when information is updated
  time_t tLastUpdated;

  /**
   * Update dCpu
   */
  int updateCpu();

  /**
   * Update dRam
   */
  int updateRam();

  /**
   * Update dDisk
   */
  int updateDisk();

  /**
   * Update dNet
   */
  int updateNet();
};

#endif