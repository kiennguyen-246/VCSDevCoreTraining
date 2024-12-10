#include "Configuration.hpp"

Configuration::Configuration() {}

Configuration::Configuration(std::string __sName, double __dCpu, double __dRam,
                             double __dDisk, double __dNet) {
  sName = __sName;
  dCpu = __dCpu;
  dRam = __dRam;
  dDisk = __dDisk;
  dNet = __dNet;
}

Configuration::Configuration(MINI_CONFIGURATION mcfg) {
  sName = mcfg.pcName;
  dCpu = mcfg.dCpu;
  dRam = mcfg.dRam;
  dDisk = mcfg.dDisk;
  dNet = mcfg.dNet;
}

Configuration::~Configuration() {}

std::string Configuration::getName() const { return sName; }

double Configuration::getCpu() const { return dCpu; }

double Configuration::getRam() const { return dRam; }

double Configuration::getDisk() const { return dDisk; }

double Configuration::getNet() const { return dNet; }

MINI_CONFIGURATION Configuration::minimize() const {
  MINI_CONFIGURATION ret;
  memset(&ret, 0, sizeof(MINI_CONFIGURATION));
  strcpy(ret.pcName, sName.c_str());
  ret.dCpu = dCpu;
  ret.dRam = dRam;
  ret.dDisk = dDisk;
  ret.dNet = dNet;
  return ret;
}

std::string Configuration::toString() const {
  std::string ret = std::format(
      "{{ \"procName\": \"{}\", \"cpu\": {:.2f}, \"ram\": {:.2f}, \"disk\": "
      "{:.2f}, \"net\": {:.2f} }}",
      sName, dCpu, dRam, dDisk, dNet);
  return ret;
}