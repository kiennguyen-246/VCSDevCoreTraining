#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP

#include <cstring>
#include <format>
#include <iostream>

typedef struct _MINI_CONFIGURATION {
  char pcName[256];
  double dCpu;
  double dRam;
  double dDisk;
  double dNet;
} MINI_CONFIGURATION, *PMINICONFIGURATION;

class Configuration {
 public:
  Configuration();

  Configuration(std::string __sName, double __dCpu, double __dRam,
                double __dDisk, double __dNet);

  Configuration(MINI_CONFIGURATION mcfg);

  ~Configuration();

  std::string getName() const;

  double getCpu() const;

  double getRam() const;

  double getDisk() const;

  double getNet() const;

  std::string toString() const;

  MINI_CONFIGURATION minimize() const;

 private:
  std::string sName;
  double dCpu;
  double dRam;
  double dDisk;
  double dNet;
};

#endif