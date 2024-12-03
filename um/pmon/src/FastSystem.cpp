#include "utils/FastSystem.hpp"

int FastSystem::system(const std::string& sCmd, std::string& sOutput) {
  std::string sResolvedCmd = Utils::resolveHomeDir(sCmd);

  // std::cout << sResolvedCmd << "\n";

  std::istringstream issCmd(sResolvedCmd);
  std::string sProg;
  std::vector<std::string> vsArgs;
  std::string sArg;
  issCmd >> sProg;
  vsArgs.push_back("/usr/bin/" + sProg);
  while (issCmd >> sArg) {
    vsArgs.push_back(sArg);
  }

  // for (auto i : vsArgs) std::cout << i << "\n";

  pid_t childPid;
  int piFds[2];
  char** ppcArgs = new char*[vsArgs.size() + 1];
  // char* ppcArgs[] = {"pgrep", "test2", NULL};
  char pcBuffer[1024];

  for (int i = 0; i < vsArgs.size(); i++) {
    ppcArgs[i] = new char[vsArgs[i].size() + 1];
    memset(ppcArgs[i], 0, sizeof(ppcArgs[i]));
    memcpy(ppcArgs[i], vsArgs[i].c_str(), vsArgs[i].size());
    // ppcArgs[i][vsArgs[i].size()] = NULL;
  }

  ppcArgs[vsArgs.size()] = NULL;
  // std::cout << std::string(ppcArgs[0]) << "\n";
  if (pipe(piFds) == -1) {
    logEvent(std::format("pipe() failed {}", errno), LOG_TYPE_ERROR);
  }

  switch (childPid = fork()) {
    case -1:
      logEvent(std::format("fork() failed {}", errno), LOG_TYPE_ERROR);
      return -1;

    case 0:
      if (close(piFds[0]) == -1) {
        logEvent(std::format("close() failed {}", errno), LOG_TYPE_ERROR);
        return errno;
      }

      if (piFds[1] != STDOUT_FILENO) {
        if (dup2(piFds[1], STDOUT_FILENO) == -1) {
          logEvent(std::format("dup2() failed {}", errno), LOG_TYPE_ERROR);
          return errno;
        }
        if (close(piFds[1] == -1)) {
          logEvent(std::format("close() failed {}", errno), LOG_TYPE_ERROR);
          return errno;
        }
      }

      execve(std::format("/usr/bin/{}", sProg).c_str(), ppcArgs, NULL);
      break;

    default:
      if (close(piFds[1]) == -1) {
        logEvent(std::format("close() failed {}", errno), LOG_TYPE_ERROR);
        return errno;
      }

      while (1) {
        memset(pcBuffer, 0, sizeof(pcBuffer));
        int iBytesRead =
            read(piFds[0], pcBuffer, sizeof(pcBuffer) * sizeof(char));
        if (iBytesRead == -1) {
          logEvent(std::format("read() failed {}", errno), LOG_TYPE_ERROR);
          return errno;
        }
        if (iBytesRead == 0) {
          break;
        }
        sOutput += std::string(pcBuffer);
      }

      for (int i = 0; i <= vsArgs.size(); i++) {
        delete[] ppcArgs[i];
      }
      delete[] ppcArgs;

      // std::cout << sOutput << "\n";

      if (waitpid(childPid, NULL, 0) == -1) {
        logEvent(std::format("Command failed"), LOG_TYPE_WARNING);
      }
      break;
  }
  return 0;
}

int FastSystem::mkdir(const std::string& sDir, const std::string& sOptions) {
  std::string sOutput;
  int iResult =
      FastSystem::system(std::format("mkdir {} {}", sDir, sOptions), sOutput);
  if (iResult) {
    logEvent(std::format("FastSystem::system() failed: {}", iResult),
             LOG_TYPE_WARNING);
    return iResult;
  }
  if (sOutput.find("mkdir") != std::string::npos) {
    logEvent(std::format("mkdir failed: {}", sOutput), LOG_TYPE_WARNING);
    return -1;
  }
  return 0;
}

int FastSystem::ls(const std::string& sDir, std::vector<std::string>& vsRet) {
  // std::cout << std::format("ls {}", sDir) << "\n";

  std::string sOutput;
  int iResult = FastSystem::system(std::format("ls {}", sDir), sOutput);
  if (iResult) {
    logEvent(std::format("FastSystem::system() failed: {}", iResult),
             LOG_TYPE_WARNING);
    return iResult;
  }

  std::istringstream issOutput(sOutput);
  std::string sName;
  while (getline(issOutput, sName)) {
    vsRet.push_back(sName);
  }
  return 0;
}

int FastSystem::pgrep(const std::string& sProcName, std::vector<int>& viRet,
                      const std::string& sOptions) {
  std::string sOutput;
  // std::cout << std::format("pgrep {} {}", sProcName, sOptions) << "\n";
  int iResult = FastSystem::system(
      std::format("pgrep {} {}", sProcName, sOptions), sOutput);

  if (iResult) {
    logEvent(std::format("FastSystem::system() failed: {}", iResult),
             LOG_TYPE_WARNING);
    return iResult;
  }

  if (sOutput.find("pgrep") != std::string::npos) {
    logEvent(std::format("pgrep failed: {}", sOutput), LOG_TYPE_WARNING);
    return -1;
  }

  std::istringstream issOutput(sOutput);
  std::string sName;
  while (getline(issOutput, sName)) {
    viRet.push_back(atoi(sName.c_str()));
  }
  return 0;
}

int FastSystem::rm(const std::string& sPath, const std::string& sOptions) {
  std::string sOutput;
  int iResult =
      FastSystem::system(std::format("rm {} {}", sPath, sOptions), sOutput);
  if (iResult) {
    logEvent(std::format("FastSystem::system() failed: {}", iResult),
             LOG_TYPE_WARNING);
    return iResult;
  }
  if (sOutput.find("rm") != std::string::npos) {
    logEvent(std::format("rm failed: {}", sOutput), LOG_TYPE_WARNING);
    return -1;
  }
  return 0;
}