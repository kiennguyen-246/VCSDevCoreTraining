#include <unistd.h>

#include <format>
#include <fstream>
#include <iostream>
#include <sstream>

const std::string TEMP_OUTPUT_FILE = "/tmp/rat/clientOutput.txt";

class CommandManager {
 private:
  static CommandManager* pcmInstance;

  CommandManager();

  ~CommandManager();

  bool runCommand(std::string sCommand, std::string* psOutput,
                  bool* pbIsTruncated);

 public:
  static CommandManager* getInstance();

  bool pwd(std::string* psOutput, bool* pbIsTruncated);

  bool cd(std::string sPath, std::string* psOutput, bool* pbIsTruncated);

  bool ls(std::string sPath, std::string* psOutput, bool* pbIsTruncated);

  bool cat(std::string sPath, std::string* psOutput, bool* pbIsTruncated);

  bool ps(std::string* psOutput, bool* pbIsTruncated);

  bool kill(std::string sProcName, std::string* psOutput, bool* pbIsTruncated);

  bool kill(unsigned int uiPid, std::string* psOutput, bool* pbIsTruncated);
};