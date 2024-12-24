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
  /**
   * Default constructor
   */
  OverloadEvent();

  /**
   * Construct an overload event from a process and a configuration
   *
   * @param proc A process object
   * @param conf A configuration object
   */
  OverloadEvent(Process& proc, Configuration& conf);

  /**
   * Default destructor
   */
  ~OverloadEvent();

  /**
   * Create a string to describe the current overload event. This string is also
   * used for sending
   */
  std::string toString();

  /**
   * Determine if a specified process violates a specified configuration
   *
   * @param proc The process
   * @param conf The configuration
   */
  static int getViolationInfo(const Process& proc, const Configuration& conf);

 private:
  // Pointer to the process object
  Process* pProc;

  // Pointer to the configuration object
  Configuration* pConf;

  // A string describing the process object
  std::string sProcInfo;

  // A string describing the configuration object
  std::string sConfInfo;

  // An integer value describing the violation type of the process in this
  // overload event, created from bitwise OR operation of
  // OVERLOAD_EVENT_VIOLATION_TYPE_XXX constants
  int overloadType;
};

#endif