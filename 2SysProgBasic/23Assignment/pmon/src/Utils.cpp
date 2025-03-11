#include <utils/Utils.hpp>

std::string Utils::resolveHomeDir(const std::string& sPath) {
  std::string sNewPath = "";
#ifdef _WIN32
  char pcHomeDir[256];
  size_t retVal = 0;
  getenv_s(&retVal, pcHomeDir, sizeof(pcHomeDir), "USERPROFILE");
  std::string sHomeDir = pcHomeDir;
#else
  std::string sHomeDir = getenv("HOME");
#endif
  for (auto i : sPath) {
    if (i == '~') {
      sNewPath += sHomeDir;
    } else {
      sNewPath += i;
    }
  }
  return sNewPath;
}

std::string Utils::wstringToString(const std::wstring& wstr) {
  std::string sRet = "";
  for (auto wc : wstr) {
    int i = 0;
    char pcRes[16];
#ifdef __linux__
    wctomb(pcRes, wc);
#endif
#ifdef _WIN32
    wctomb_s(&i, pcRes, 16, wc);
#endif
    sRet.push_back(pcRes[0]);
  }
  return sRet;
}