#include <iostream>
#include <cstring>

#include "EventLogger.hpp"
#include "WinSafe.hpp"

namespace Utils {
std::string resolveHomeDir(const std::string& sPath);

std::string wstringToString(const std::wstring& wstr);
};