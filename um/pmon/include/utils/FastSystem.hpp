#ifndef FAST_SYSTEM_HPP
#define FAST_SYSTEM_HPP

#ifdef __linux__
#include <sys/wait.h>
#include <unistd.h>
#endif

#include <cstdlib>
#include <cstring>
#include <format>
#include <iostream>
#include <sstream>
#include <vector>

#include "utils/EventLogger.hpp"
#include "utils/Utils.hpp"

namespace FastSystem {
int system(const std::string& sCmd, std::string& sOutput);

int mkdir(const std::string& sDir, const std::string& sOptions = "");

int ls(const std::string& sDir, std::vector<std::string>& vsRet);

int pgrep(const std::string& sProcName, std::vector<int>& viRet,
          const std::string& sOptions = "");

int rm(const std::string& sPath, const std::string& sOptions = "");
};  // namespace FastSystem

#endif