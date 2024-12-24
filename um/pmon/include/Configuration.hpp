#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP

#ifdef _WIN32
#include <Windows.h>
#endif

#include <cstring>
#include <format>
#include <iostream>

#include "utils/WinSafe.hpp"

// Minimized version of a configuration object, used for sending
typedef struct _MINI_CONFIGURATION {
  // Buffer to the name of the process
  char pcName[256];

  // Process CPU ultilization limit
  double dCpu;

  // Process RAM ultilization limit
  double dRam;

  // Process disk ultilization limit
  double dDisk;

  // Process network ultilization limit
  double dNet;
} MINI_CONFIGURATION, *PMINICONFIGURATION;

// Configuration object
class Configuration {
 public:
  /**
   * Default constructor.
   */
  Configuration();

  /**
   * Construct a configuration object from a process name and limit values.
   *
   * @param __sName Name of the process
   * @param __dCpu Process CPU ultilization limit
   * @param __dRam Process RAM ultilization limit
   * @param __dDisk Process disk ultilization limit
   * @param __dNet Process network ultilization limit
   *
   */
  Configuration(std::string __sName, double __dCpu, double __dRam,
                double __dDisk, double __dNet);

  /**
   * Construct a configuration object from a minimized version.
   *
   * @param mcfg Minimized configuration object
   *
   */
  Configuration(MINI_CONFIGURATION mcfg);

  /**
   * Default destructor.
   */
  ~Configuration();

  /**
   * Get the name of the processes that should be refered to by this
   * configuration
   */
  std::string getName() const;

  /**
   * Get the CPU utilization limit of this configuration
   */
  double getCpu() const;

  /**
   * Get the RAM utilization limit of this configuration
   */
  double getRam() const;

  /**
   * Get the disk utilization limit of this configuration
   */
  double getDisk() const;

  /**
   * Get the network utilization limit of this configuration
   */
  double getNet() const;

  /**
   * Get a string to describe the configuration
   */
  std::string toString() const;

  /**
   * Minimize the configuration
   */
  MINI_CONFIGURATION minimize() const;

 private:
  // The name of the processes that should be refered to by this configuration
  std::string sName;

  // The CPU utilization limit of this configuration
  double dCpu;

  // The RAM utilization limit of this configuration
  double dRam;

  // The disk utilization limit of this configuration
  double dDisk;

  // The network utilization limit of this configuration
  double dNet;
};

#endif