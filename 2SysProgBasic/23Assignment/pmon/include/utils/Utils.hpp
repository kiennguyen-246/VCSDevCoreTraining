#include <cstring>
#include <iostream>

#include "EventLogger.hpp"
#include "WinSafe.hpp"

// Utility helper functions
namespace Utils {

/**
 * Resolve "~"" to the Linux home directory or the directory refered to by
 * Windows USERPROFILE environment variable
 *
 * @param sPath The path containing ~
 */
std::string resolveHomeDir(const std::string& sPath);

/**
 * Convert a wstring object to a string object
 *
 * @param wstr The wstring object
 */
std::string wstringToString(const std::wstring& wstr);
};  // namespace Utils