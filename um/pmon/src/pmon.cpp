#include <sys/wait.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#include "pmon/PmonController.hpp"

int main(int argc, char* argv[]) {
  // Process* pProc = new Process(3244);
  // Configuration* pConf = new Configuration("code", 100, 30, 100, 100);

  // while (1) {
  //   sleep(3);
  //   pProc->updateInfo();
  //   std::cout << pProc->toString() << "\n";
  //   if (OverloadEvent::getViolationInfo(*pProc, *pConf)) {
  //     Event* pEvt = new OverloadEvent(*pProc, *pConf);
  //     std::cout << pEvt->toString() << "\n";
  //     delete pEvt;
  //   }
  // }

  // delete pConf;
  // delete pProc;

  // std::string sCmd = "pgrep -x test2";
  // std::string sOutput = "";
  // FastSystem::system(sCmd, sOutput);
  // std::cout << sOutput;

  // std::vector<int> viOutput;
  // FastSystem::pgrep("test1", viOutput, "-x");
  // for (auto i : viOutput) std::cout << i << "\n";

  // std::vector<std::string> vsOutput;
  // FastSystem::ls("~", vsOutput);
  // for (auto i : vsOutput) std::cout << i << "\n";

  // system("/usr/bin/pgrep test2");

  // execl("/usr/bin/pgrep", "/usr/bin/pgrep", "test2", "-x");

  // char arg1[] = "/usr/bin/pgrep";
  // char arg2[] = "test2";
  // char arg3[] = "-c";
  // char* args[] = {arg1, arg2, arg3, NULL};
  // execve("/usr/bin/pgrep", args, NULL);

  auto pPmonControllerInstance = PmonController::getInstance();
  pPmonControllerInstance->run();

  return 0;
}