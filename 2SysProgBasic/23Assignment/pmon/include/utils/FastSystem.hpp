#ifndef FAST_SYSTEM_HPP
#define FAST_SYSTEM_HPP

#ifdef __linux__
#include <sys/wait.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <format>
#include <iostream>
#include <sstream>
#include <vector>

#include "utils/EventLogger.hpp"
#include "utils/Utils.hpp"

// Fast Linux system() operations by running the programs directly and obtaining
// result through pipes (skipping controlling terminal)
namespace FastSystem {
/**
 * Alternative to system()
 *
 * @param sCmd The command
 * @param sOutput The result
 *
 */
int system(const std::string& sCmd, std::string& sOutput);

/**
 * Alternative to system("mkdir...")
 *
 * @param sDir The directory needs creating
 * @param sOptions Options for the command
 *
 */
int mkdir(const std::string& sDir, const std::string& sOptions = "");

/**
 * Alternative to system("ls...")
 *
 * @param sDir The directory needs listing
 * @param vsRet A vector containing entries listed by ls
 *
 */
int ls(const std::string& sDir, std::vector<std::string>& vsRet);

/**
 * Alternative to system("pgrep...")
 *
 * @param sProcName The process name needs querying
 * @param viRet List of PIDs returned
 * @param sOptions Options for the command
 *
 */
int pgrep(const std::string& sProcName, std::vector<int>& viRet,
          const std::string& sOptions = "");

/**
 * Alternative to system("rm...")
 *
 * @param sPath The path to the file needs deleting
 * @param sOptions Options for the command
 *
 */
int rm(const std::string& sPath, const std::string& sOptions = "");
};  // namespace FastSystem

#endif

#endif