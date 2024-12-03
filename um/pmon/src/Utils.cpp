#include <utils/Utils.hpp>

std::string Utils::resolveHomeDir(const std::string& sPath) {
  std::string sNewPath = "";
  std::string sHomeDir = std::string(getenv("HOME"));
  for (auto i : sPath) {
    if (i == '~') {
      sNewPath += sHomeDir;
    } else {
      sNewPath += i;
    }
  }
  return sNewPath;
}